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

#include "Windows10Colors.h"

#include <CommCtrl.h>
#include <windowsx.h>
#include <winreg.h>

static const wchar_t autostart_registry_path[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const wchar_t autostart_registry_key[] = L"HDRTray";

NotifyIcon::NotifyIcon(HWND hwnd)
{
    notify_template = NOTIFYICONDATAW { sizeof(NOTIFYICONDATAW) };
    notify_template.hWnd = hwnd;
    notify_template.uID = 0;
    notify_template.uFlags = NIF_MESSAGE | NIF_GUID | NIF_SHOWTIP;
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

bool NotifyIcon::Add()
{
    FetchHDRStatus();
    FetchDarkMode();

    auto notify_add = notify_template;
    notify_add.hIcon = GetCurrentIconSet().hdr_off;
    LoadStringW(hInst, IDS_APP_TITLE, notify_add.szTip, ARRAYSIZE(notify_add.szTip));
    notify_add.uFlags |= NIF_ICON | NIF_TIP | NIF_SHOWTIP;
    if(!Shell_NotifyIconW(NIM_ADD, &notify_add))
        return false;

    auto notify_setversion = notify_template;
    notify_setversion.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIconW(NIM_SETVERSION, &notify_setversion);

    UpdateIcon();
    return true;
}

void NotifyIcon::Remove()
{
    auto notify_delete = notify_template;
    Shell_NotifyIconW(NIM_DELETE, &notify_delete);
}

void NotifyIcon::UpdateHDRStatus()
{
    FetchHDRStatus();
    UpdateIcon();
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

// Quote the executable path
static std::wstring get_autostart_value()
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

static bool is_autostart_enabled(HKEY key_autostart, const wchar_t* autostart_value)
{
    DWORD value_type = 0;
    DWORD value_size = 0;
    auto query_value_res = RegQueryValueExW(key_autostart, autostart_registry_key, nullptr, &value_type, nullptr, &value_size);
    bool has_auto_start = (query_value_res == ERROR_SUCCESS) || (query_value_res == ERROR_MORE_DATA);
    if(has_auto_start)
        has_auto_start &= value_type == REG_SZ;
    if(has_auto_start)
    {
        DWORD value_len = value_size / sizeof(WCHAR);
        DWORD buf_size = (value_len + 1) * sizeof(WCHAR);
        auto* buf = reinterpret_cast<WCHAR*>(_alloca(buf_size));
        if (RegQueryValueExW(key_autostart, autostart_registry_key, nullptr, nullptr, reinterpret_cast<BYTE*>(buf),
                             &buf_size)
            == ERROR_SUCCESS) {
            buf[value_len] = 0;
            has_auto_start = _wcsicmp(buf, autostart_value) == 0;
        } else {
            has_auto_start = false;
        }
    }

    return has_auto_start;
}

void NotifyIcon::ToggleAutostartEnabled()
{
    HKEY key_autostart = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, autostart_registry_path, 0, nullptr, 0,
                        KEY_READ | KEY_WRITE | KEY_QUERY_VALUE | KEY_SET_VALUE, nullptr, &key_autostart, nullptr)
        != ERROR_SUCCESS)
        return;

    auto autostart_value = get_autostart_value();

    auto has_auto_start = is_autostart_enabled(key_autostart, autostart_value.c_str());

    bool new_auto_start = !has_auto_start;
    if(new_auto_start) {
        RegSetValueExW(key_autostart, autostart_registry_key, 0, REG_SZ, reinterpret_cast<const BYTE*>(autostart_value.c_str()),
                       static_cast<DWORD>((autostart_value.size() + 1) * sizeof(WCHAR)));
    } else {
        RegDeleteValueW(key_autostart, autostart_registry_key);
    }

    RegCloseKey(key_autostart);
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
    mii.fState = IsAutostartEnabled() ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfoW(popup_menu, IDM_AUTOSTART, false, &mii);

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

bool NotifyIcon::IsAutostartEnabled() const
{
    HKEY key_autostart = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, autostart_registry_path, 0, KEY_READ | KEY_QUERY_VALUE, &key_autostart) != ERROR_SUCCESS)
        return false;

    auto autostart_value = get_autostart_value();

    auto has_auto_start = is_autostart_enabled(key_autostart, autostart_value.c_str());

    RegCloseKey(key_autostart);

    return has_auto_start;
}
