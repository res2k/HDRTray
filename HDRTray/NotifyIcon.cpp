/*
    HDRTray, a notification icon for the "Use HDR" option
    Copyright (C) 2022 Frank Richter

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "NotifyIcon.hpp"

#include "Resource.h"
#include "WinVerCheck.hpp"

#include "Windows10Colors.h"

#include <CommCtrl.h>
#include <windowsx.h>
#include <winreg.h>

/*
    Enabling dark mode based on this information:
    https://gist.github.com/rounk-ctrl/b04e5622e30e0d62956870d5c22b7017
    https://stackoverflow.com/questions/75835069/dark-system-contextmenu-in-window
*/

enum class PreferredAppMode { Default, AllowDark, ForceDark, ForceLight, Max };
using PFN_SetPreferredAppMode = PreferredAppMode(WINAPI*)(PreferredAppMode appMode);
using PFN_FlushMenuThemes = void(WINAPI*)();

static PFN_SetPreferredAppMode SetPreferredAppMode;
static PFN_FlushMenuThemes FlushMenuThemes;
static bool has_dark_mode_support;

static void InitDarkModeSupport()
{
    // This stuff only works with Windows 1903+
    if (!IsWindows10_1903OrGreater())
        return;
    // Already initialized?
    if (SetPreferredAppMode)
        return;

    HMODULE uxtheme = LoadLibraryW(L"uxtheme.dll");
    SetPreferredAppMode = reinterpret_cast<PFN_SetPreferredAppMode>(GetProcAddress(uxtheme, MAKEINTRESOURCEA(135)));
    FlushMenuThemes = reinterpret_cast<PFN_FlushMenuThemes>(GetProcAddress(uxtheme, MAKEINTRESOURCEA(136)));

    has_dark_mode_support = SetPreferredAppMode && FlushMenuThemes;
}

static const wchar_t loginstartup_registry_path[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const wchar_t loginstartup_registry_key[] = L"HDRTray";

// Wraps Shell_NotifyIconW(), prints to debug output in case of a failure
static BOOL wrap_Shell_NotifyIconW(DWORD message, NOTIFYICONDATAW* data)
{
    auto result = Shell_NotifyIconW(message, data);
    if (result)
        return result;

    const wchar_t* msg_str = nullptr;
    switch (message) {
    case NIM_ADD:
        msg_str = L"NIM_ADD";
        break;
    case NIM_MODIFY:
        msg_str = L"NIM_MODIFY";
        break;
    case NIM_DELETE:
        msg_str = L"NIM_DELETE";
        break;
    case NIM_SETFOCUS:
        msg_str = L"NIM_SETFOCUS";
        break;
    case NIM_SETVERSION:
        msg_str = L"NIM_SETVERSION";
        break;
    }

    wchar_t debug_message[256];
    if (msg_str) {
        swprintf_s(debug_message, L"Shell_NotifyIconW(%ls) failed :(\n", msg_str);
    } else {
        swprintf_s(debug_message, L"Shell_NotifyIconW(%d) failed :(\n", message);
    }
    OutputDebugStringW(debug_message);
    return result;
}

NotifyIcon::NotifyIcon(HWND hwnd)
{
    InitDarkModeSupport();

    notify_template = NOTIFYICONDATAW { sizeof(NOTIFYICONDATAW) };
    notify_template.hWnd = hwnd;
    notify_template.uID = 0;
    notify_template.uFlags = NIF_MESSAGE | NIF_SHOWTIP;
    notify_template.uCallbackMessage = MESSAGE;

    for (int i = 0; i < numIconsets; i++) {
        LoadIconMetric(hInst, MAKEINTRESOURCEW(IDI_HDR_ON_DARKMODE + i), LIM_SMALL, &icons[i].hdr_on);
        LoadIconMetric(hInst, MAKEINTRESOURCEW(IDI_HDR_OFF_DARKMODE + i), LIM_SMALL, &icons[i].hdr_off);
    }
    popup_menu = LoadMenuW(hInst, MAKEINTRESOURCEW(IDC_TRAYPOPUP));
}

NotifyIcon::~NotifyIcon()
{
    for (int i = 0; i < numIconsets; i++) {
        DestroyIcon(icons[i].hdr_on);
        DestroyIcon(icons[i].hdr_off);
    }
    DestroyMenu(popup_menu);
}

bool NotifyIcon::WasAdded() const
{
    return added;
}

bool NotifyIcon::Add()
{
    FetchHDRStatus();
    FetchDarkMode();

    auto notify_add = notify_template;
    notify_add.hIcon = GetCurrentIconSet().hdr_off;
    LoadStringW(hInst, IDS_APP_TITLE, notify_add.szTip, ARRAYSIZE(notify_add.szTip));
    notify_add.uFlags |= NIF_ICON | NIF_TIP | NIF_SHOWTIP;
    if(!wrap_Shell_NotifyIconW(NIM_ADD, &notify_add))
        return false;

    auto notify_setversion = notify_template;
    notify_setversion.uVersion = NOTIFYICON_VERSION_4;
    wrap_Shell_NotifyIconW(NIM_SETVERSION, &notify_setversion);

    UpdateIcon();
    added = true;
    return true;
}

void NotifyIcon::Remove()
{
    auto notify_delete = notify_template;
    wrap_Shell_NotifyIconW(NIM_DELETE, &notify_delete);
    added = false;
}

bool NotifyIcon::UpdateHDRStatus()
{
    auto prev_hdr_status = hdr_status;
    FetchHDRStatus();
    if (prev_hdr_status != hdr_status)
    {
        UpdateIcon();
        return true;
    }
    return false;
}

void NotifyIcon::UpdateDarkMode()
{
    FetchDarkMode();
    UpdateIcon();
}

LRESULT NotifyIcon::HandleMessage(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    auto event = LOWORD(lParam);
    switch(event)
    {
    case WM_CONTEXTMENU:
        PopupIconMenu(hWnd, POINT { GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam) });
        break;
    case NIN_KEYSELECT:
    case NIN_SELECT:
        ToggleHDR();
        break;
    }
    return 0;
}

bool NotifyIcon::HandleCommand(int command)
{
    switch (command)
    {
    case IDM_ENABLE_HDR:
        ToggleHDR();
        return true;
    case IDM_LOGIN_STARTUP:
        ToggleLoginStartupEnabled();
        return true;
    }
    return false;
}

// Quote the executable path
static std::wstring get_loginstartup_value()
{
    wchar_t* exe_path = nullptr;
    _get_wpgmptr(&exe_path);

    std::wstring result;
    result.reserve(wcslen(exe_path) + 2);
    result.push_back('"');
    result.append(exe_path);
    result.push_back('"');
    return result;
}

static bool is_loginstartup_enabled(HKEY key_loginstartup, const wchar_t* loginstartup_value)
{
    DWORD value_type = 0;
    DWORD value_size = 0;
    auto query_value_res = RegQueryValueExW(key_loginstartup, loginstartup_registry_key, nullptr, &value_type, nullptr, &value_size);
    bool has_auto_start = (query_value_res == ERROR_SUCCESS) || (query_value_res == ERROR_MORE_DATA);
    if(has_auto_start)
        has_auto_start &= value_type == REG_SZ;
    if(has_auto_start)
    {
        DWORD value_len = value_size / sizeof(WCHAR);
        DWORD buf_size = (value_len + 1) * sizeof(WCHAR);
        auto* buf = reinterpret_cast<WCHAR*>(_alloca(buf_size));
        if (RegQueryValueExW(key_loginstartup, loginstartup_registry_key, nullptr, nullptr, reinterpret_cast<BYTE*>(buf),
                             &buf_size)
            == ERROR_SUCCESS) {
            buf[value_len] = 0;
            has_auto_start = _wcsicmp(buf, loginstartup_value) == 0;
        } else {
            has_auto_start = false;
        }
    }

    return has_auto_start;
}

void NotifyIcon::ToggleLoginStartupEnabled()
{
    HKEY key_loginstartup = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, loginstartup_registry_path, 0, nullptr, 0,
                        KEY_READ | KEY_WRITE | KEY_QUERY_VALUE | KEY_SET_VALUE, nullptr, &key_loginstartup, nullptr)
        != ERROR_SUCCESS)
        return;

    auto loginstartup_value = get_loginstartup_value();

    auto has_auto_start = is_loginstartup_enabled(key_loginstartup, loginstartup_value.c_str());

    bool new_auto_start = !has_auto_start;
    if(new_auto_start) {
        RegSetValueExW(key_loginstartup, loginstartup_registry_key, 0, REG_SZ, reinterpret_cast<const BYTE*>(loginstartup_value.c_str()),
                       static_cast<DWORD>((loginstartup_value.size() + 1) * sizeof(WCHAR)));
    } else {
        RegDeleteValueW(key_loginstartup, loginstartup_registry_key);
    }

    RegCloseKey(key_loginstartup);
}

void NotifyIcon::ToggleHDR()
{
    /* Toggling HDR moves the mouse cursor to the screen center,
     * so save & restore it's position */
    POINT mouse_pos;
    bool has_mouse_pos = GetCursorPos(&mouse_pos);

    auto new_status = hdr::ToggleHDRStatus();

    if(new_status) {
        hdr_status = *new_status;
        UpdateIcon();
    } else {
        // Pop up error balloon if toggle failed
        auto notify_balloon_tip = notify_template;
        notify_balloon_tip.uFlags |= NIF_INFO | NIF_REALTIME;
        LoadStringW(hInst, IDS_TOGGLE_HDR_ERROR, notify_balloon_tip.szInfo, ARRAYSIZE(notify_balloon_tip.szInfo));
        notify_balloon_tip.dwInfoFlags = NIIF_ERROR;
        Shell_NotifyIconW(NIM_MODIFY, &notify_balloon_tip);
    }

    if(has_mouse_pos)
        SetCursorPos(mouse_pos.x, mouse_pos.y);
}

void NotifyIcon::PopupIconMenu(HWND hWnd, POINT pos)
{
    // needed to clicking "outside" the menu works
    SetForegroundWindow(hWnd);

    MENUITEMINFOW mii = { sizeof(MENUITEMINFOW) };
    mii.fMask = MIIM_STATE;
    mii.fState = IsLoginStartupEnabled() ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfoW(popup_menu, IDM_LOGIN_STARTUP, false, &mii);

    wchar_t str_hdr_unsupported[256];
    mii = { sizeof(MENUITEMINFOW) };
    if(hdr_status == hdr::Status::Unsupported) {
        LoadStringW(hInst, IDS_HDR_UNSUPPORTED, str_hdr_unsupported, ARRAYSIZE(str_hdr_unsupported));
        mii.fMask = MIIM_STATE | MIIM_TYPE ;
        mii.fState = MFS_DISABLED;
        mii.fType = MFT_STRING;
        mii.dwTypeData = str_hdr_unsupported;
    } else {
        mii.fMask = MIIM_STATE;
        mii.fState = (hdr_status == hdr::Status::On ? MFS_CHECKED : MFS_UNCHECKED) | MFS_DEFAULT;
    }
    SetMenuItemInfoW(popup_menu, IDM_ENABLE_HDR, false, &mii);

    bool menu_right_align = GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0;
    DWORD flags = TPM_RIGHTBUTTON
        | (menu_right_align ? TPM_HORNEGANIMATION | TPM_RIGHTALIGN : TPM_HORPOSANIMATION | TPM_LEFTALIGN);
    TrackPopupMenuEx(GetSubMenu(popup_menu, 0), flags, pos.x, pos.y, hWnd, nullptr);
}
const NotifyIcon::Icons& NotifyIcon::GetCurrentIconSet() const
{
    return icons[dark_mode_icons ? iconsetDarkMode : iconsetLightMode];
}

void NotifyIcon::FetchHDRStatus()
{
    this->hdr_status = hdr::GetWindowsHDRStatus();
}

void NotifyIcon::FetchDarkMode()
{
    windows10colors::SysPartsMode sys_parts_coloring;
    if(FAILED(GetSysPartsMode(sys_parts_coloring)))
        return;

    // In both "dark" and "accented" modes the task bar is dark enough to require light text
    dark_mode_icons = sys_parts_coloring != windows10colors::SysPartsMode::Light;

    if (has_dark_mode_support) {
        // Make context menu popup mode match task bar mode
        SetPreferredAppMode(dark_mode_icons ? PreferredAppMode::ForceDark : PreferredAppMode::ForceLight);
        FlushMenuThemes();
    }
}

void NotifyIcon::UpdateIcon()
{
    auto notify_mod = notify_template;
    notify_mod.uFlags |= NIF_ICON | NIF_TIP;
    switch(hdr_status)
    {
    default:
    case hdr::Status::Unsupported:
        notify_mod.hIcon = GetCurrentIconSet().hdr_off;
        LoadStringW(hInst, IDS_HDR_UNSUPPORTED, notify_mod.szTip, ARRAYSIZE(notify_mod.szTip));
        break;
    case hdr::Status::Off:
        notify_mod.hIcon = GetCurrentIconSet().hdr_off;
        LoadStringW(hInst, IDS_HDR_OFF, notify_mod.szTip, ARRAYSIZE(notify_mod.szTip));
        break;
    case hdr::Status::On:
        notify_mod.hIcon = GetCurrentIconSet().hdr_on;
        LoadStringW(hInst, IDS_HDR_ON, notify_mod.szTip, ARRAYSIZE(notify_mod.szTip));
        break;
    }
    Shell_NotifyIconW(NIM_MODIFY, &notify_mod);

}

bool NotifyIcon::IsLoginStartupEnabled() const
{
    HKEY key_loginstartup = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, loginstartup_registry_path, 0, KEY_READ | KEY_QUERY_VALUE, &key_loginstartup) != ERROR_SUCCESS)
        return false;

    auto loginstartup_value = get_loginstartup_value();

    auto has_auto_start = is_loginstartup_enabled(key_loginstartup, loginstartup_value.c_str());

    RegCloseKey(key_loginstartup);

    return has_auto_start;
}
