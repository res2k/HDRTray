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

#include <windowsx.h>

// {2D4645CF-59A2-4566-9D8B-86017A629D0A}
const GUID NotifyIcon::guid = { 0x2d4645cf, 0x59a2, 0x4566, { 0x9d, 0x8b, 0x86, 0x1, 0x7a, 0x62, 0x9d, 0xa } };

NotifyIcon::NotifyIcon(HWND hwnd)
{
    notify_template = NOTIFYICONDATAW { sizeof(NOTIFYICONDATAW) };
    notify_template.hWnd = hwnd;
    notify_template.uFlags = NIF_MESSAGE | NIF_GUID;
    notify_template.uCallbackMessage = MESSAGE;
    notify_template.guidItem = guid;

    for (int i = 0; i < numIconsets; i++) {
        icons[i].hdr_on = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_HDR_ON_DARKMODE + i));
        icons[i].hdr_off = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_HDR_OFF_DARKMODE + i));
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
    notify_add.uFlags |= NIF_ICON | NIF_TIP;
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
    case NIN_KEYSELECT:
    case NIN_SELECT:
        PopupIconMenu(hWnd, POINT { GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam) });
        break;
    }
    return 0;
}

void NotifyIcon::PopupIconMenu(HWND hWnd, POINT pos)
{
    // needed to clicking "outside" the menu works
    SetForegroundWindow(hWnd);

    bool menu_right_align = GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0;
    TrackPopupMenuEx(GetSubMenu(popup_menu, 0),
                     menu_right_align ? TPM_HORNEGANIMATION | TPM_RIGHTALIGN : TPM_HORPOSANIMATION | TPM_LEFTALIGN,
                     pos.x, pos.y, hWnd, nullptr);
}
const NotifyIcon::Icons& NotifyIcon::GetCurrentIconSet() const
{
    return icons[dark_mode_icons ? iconsetDarkMode : iconsetLightMode];
}

void NotifyIcon::FetchHDRStatus()
{
    // Compute a singular status across all monitors
    int single_status = static_cast<int>(hdr::Status::Unsupported);
    for(const auto& display_status : hdr::GetWindowsHDRStatus()) {
        single_status = std::max(single_status, static_cast<int>(display_status.second));
    }
    this->hdr_status = static_cast<hdr::Status>(single_status);
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
