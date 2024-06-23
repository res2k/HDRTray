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

#ifndef NOTIFYICON_HPP_
#define NOTIFYICON_HPP_

#include "framework.h"
#include "HDR.h"

#include <shellapi.h>

class NotifyIcon
{
    bool added = false;
    NOTIFYICONDATAW notify_template;

    enum { iconsetDarkMode = 0, iconsetLightMode, numIconsets };
    struct Icons
    {
        HICON hdr_on;
        HICON hdr_off;
    };
    Icons icons[numIconsets];
    HMENU popup_menu;

    bool dark_mode_icons = false;
    hdr::Status hdr_status = hdr::Status::Unsupported;

public:
    NotifyIcon(HWND hwnd);
    ~NotifyIcon();

    bool WasAdded() const;
    bool Add();
    void Remove();

    bool UpdateHDRStatus();
    void UpdateDarkMode();

    LRESULT HandleMessage(HWND hWnd, WPARAM wParam, LPARAM lParam);

    enum { MESSAGE = WM_USER + 11 };

    // Handle (some) context menu commands
    bool HandleCommand(int command);

protected:
    void PopupIconMenu(HWND hWnd, POINT pos);

    const Icons& GetCurrentIconSet() const;
    void FetchHDRStatus();
    void FetchDarkMode();
    void UpdateIcon();

    bool IsAutostartEnabled() const;
    void ToggleAutostartEnabled();
    void ToggleHDR();
};

#endif // NOTIFYICON_HPP_
