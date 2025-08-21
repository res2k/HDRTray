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

#include "DisplayConfigWatcher.hpp"
#include "framework.h"
#include "HDRTray.h"
#include "HDR.h"
#include "LoginStartupConfig.hpp"
#include "NotifyIcon.hpp"
#include "WinVerCheck.hpp"

#include <memory>
#include <utility>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
static const wchar_t szWindowClass[] = L"HDRTrayWindow";  // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if(IsWindows10_1709OrGreater())
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    else
        SetProcessDPIAware();

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

    // if Windows 10 < version 1809 refuse to start
    if (!IsWindows10_1809OrGreater()) {
        const wchar_t* string_data = nullptr;
        int message_len = LoadStringW(hInst, IDS_WINDOWS_TOO_OLD, (LPWSTR)&string_data, 0);
        auto message = std::wstring(string_data, message_len);
        MessageBoxW(nullptr, message.c_str(), szTitle, MB_OK | MB_ICONERROR);
        return 1;
    }

    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return 2;
    }

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = { sizeof(WNDCLASSEX) };

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszClassName  = szWindowClass;

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   // Position window at (0,0) so it's always on the primary monitor
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      0, 0, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   return TRUE;
}

static std::unique_ptr<NotifyIcon> notify_icon;
static std::unique_ptr<DisplayConfigWatcher> display_config_watcher;
static UINT msg_TaskbarCreated;
static unsigned hdr_status_check_count;

enum { TIMER_ID_WAIT_TASKBAR_CREATED = 1, TIMER_ID_RECHECK_HDR_STATUS = 2 };

static void HandleTimer(HWND hWnd, int id)
{
    switch(id)
    {
    case TIMER_ID_WAIT_TASKBAR_CREATED:
        KillTimer(hWnd, TIMER_ID_WAIT_TASKBAR_CREATED);
        // No TaskbarCreated was received, exit
        if (!notify_icon->WasAdded())
            DestroyWindow(hWnd);
        break;
    case TIMER_ID_RECHECK_HDR_STATUS:
        if (hdr_status_check_count > 0) {
            hdr_status_check_count--;
            if (notify_icon->UpdateHDRStatus())
                hdr_status_check_count = 0;
        } else
            KillTimer(hWnd, TIMER_ID_RECHECK_HDR_STATUS);
    }
}

#define WM_HDRTRAY_UPDATE_STATUS    WM_USER + 1

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        msg_TaskbarCreated = RegisterWindowMessage(L"TaskbarCreated");
        notify_icon.reset(new NotifyIcon(hWnd));
        if (!notify_icon->Add())
        {
            // Set up a timer, this is the amount of time we wait for TaskbarCreated
            SetTimer(hWnd, TIMER_ID_WAIT_TASKBAR_CREATED, 30000, nullptr);
        }
        display_config_watcher.reset(
            new DisplayConfigWatcher([=]() { PostMessage(hWnd, WM_HDRTRAY_UPDATE_STATUS, 0, 0); }));
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            if (notify_icon->HandleCommand(wmId))
                break;
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_DISPLAYCHANGE:
        // Position window at (0,0) so it's always on the primary monitor
        SetWindowPos(hWnd, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        if (!notify_icon->UpdateHDRStatus())
        {
            /* HDR status doesn't seem to be always immediately up-to-date when receiving
             * WM_DISPLAYCHANGE, so periodically re-check it over a short duration */
            hdr_status_check_count = 10;
            SetTimer(hWnd, TIMER_ID_RECHECK_HDR_STATUS, 500, nullptr);
        }
        break;
    case WM_SETTINGCHANGE:
        notify_icon->UpdateDarkMode();
        break;
    case WM_DESTROY:
        notify_icon->Remove();
        notify_icon.reset();
        display_config_watcher.reset();
        PostQuitMessage(0);
        break;
    case NotifyIcon::MESSAGE:
        return notify_icon->HandleMessage(hWnd, wParam, lParam);
    case WM_HDRTRAY_UPDATE_STATUS:
        notify_icon->UpdateHDRStatus();
        break;
    case WM_TIMER:
        HandleTimer(hWnd, wParam);
        break;
    default:
        if (message == msg_TaskbarCreated) {
            // Taskbar was created (Explorer restart, DPI change), so re-create notify icon
            notify_icon->Remove();
            if (!notify_icon->Add())
                DestroyWindow(hWnd);
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
