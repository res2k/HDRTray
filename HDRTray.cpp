// HDRTray.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "HDRTray.h"
#include "HDR.h"

#include "Windows10Colors.h"

#include <shellapi.h>
#include <windowsx.h>

#include <memory>
#include <utility>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
static const wchar_t szWindowClass[] = L"HDRTrayWindow";  // the main window class name

class NotifyIcon
{
    static const GUID guid;
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

    bool Add();
    void Remove();

    void UpdateHDRStatus();
    void UpdateDarkMode();

    LRESULT HandleMessage(HWND hWnd, WPARAM wParam, LPARAM lParam);

    enum { MESSAGE = WM_USER + 11 };

protected:
    void PopupIconMenu(HWND hWnd, POINT pos);

    const Icons& GetCurrentIconSet() const;
    void FetchHDRStatus();
    void FetchDarkMode();
    void UpdateIcon();
};
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

    SetProcessDPIAware();

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   return TRUE;
}

static std::unique_ptr<NotifyIcon> notify_icon;

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
        notify_icon.reset(new NotifyIcon(hWnd));
        notify_icon->Add();
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
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
        notify_icon->UpdateHDRStatus();
        break;
    case WM_SETTINGCHANGE:
        notify_icon->UpdateDarkMode();
        break;
    case WM_DESTROY:
        notify_icon->Remove();
        notify_icon.reset();
        PostQuitMessage(0);
        break;
    case NotifyIcon::MESSAGE:
        return notify_icon->HandleMessage(hWnd, wParam, lParam);
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
