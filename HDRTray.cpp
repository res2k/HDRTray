// HDRTray.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "HDRTray.h"
#include "HDR.h"

#include <shellapi.h>

#include <memory>
#include <utility>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

class NotifyIcon
{
    static const GUID guid;
    NOTIFYICONDATAW notify_template;

    HICON icon_hdr_on;
    HICON icon_hdr_off;
    HMENU popup_menu;

public:
    NotifyIcon(HWND hwnd);
    ~NotifyIcon();

    bool Add();
    void Remove();

    void SetFromHDRStatus(const hdr::monitor_status_vec& hdr_status);

    LRESULT HandleMessage(HWND hWnd, WPARAM wParam, LPARAM lParam);

    enum { MESSAGE = WM_USER + 11 };

protected:
    void PopupIconMenu(HWND hWnd);

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

    icon_hdr_on = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_HDR_ON));
    icon_hdr_off = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_HDR_OFF));
    popup_menu = LoadMenuW(hInst, MAKEINTRESOURCEW(IDC_TRAYPOPUP));
}

NotifyIcon::~NotifyIcon()
{
    DestroyIcon(icon_hdr_on);
    DestroyIcon(icon_hdr_off);
    DestroyMenu(popup_menu);
}

bool NotifyIcon::Add()
{
    auto notify_add = notify_template;
    notify_add.hIcon = icon_hdr_off;
    notify_add.uFlags |= NIF_ICON;
    if(!Shell_NotifyIconW(NIM_ADD, &notify_add))
        return false;

    auto notify_setversion = notify_template;
    notify_setversion.uVersion = NOTIFYICON_VERSION_4;
    return Shell_NotifyIconW(NIM_SETVERSION, &notify_setversion);
}

void NotifyIcon::Remove()
{
    auto notify_delete = notify_template;
    Shell_NotifyIconW(NIM_DELETE, &notify_delete);
}

void NotifyIcon::SetFromHDRStatus(const hdr::monitor_status_vec& hdr_status)
{
    // Compute a singular status across all monitors
    int single_status = static_cast<int>(hdr::Status::Unsupported);
    for(const auto& display_status : hdr_status) {
        single_status = std::max(single_status, static_cast<int>(display_status.second));
    }

    auto notify_mod = notify_template;
    //notify_add.hIcon = icon_hdr_off;
    notify_mod.uFlags |= NIF_ICON;
    switch(static_cast<hdr::Status>(single_status))
    {
    default:
    case hdr::Status::Unsupported:
    case hdr::Status::Off:
        notify_mod.hIcon = icon_hdr_off;
        break;
    case hdr::Status::On:
        notify_mod.hIcon = icon_hdr_on;
        break;
    }
    Shell_NotifyIconW(NIM_MODIFY, &notify_mod);
}

LRESULT NotifyIcon::HandleMessage(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    auto event = LOWORD(lParam);
    switch(event)
    {
    case WM_CONTEXTMENU:
    case NIN_KEYSELECT:
    case NIN_SELECT:
        PopupIconMenu(hWnd);
        break;
    }
    return 0;
}

void NotifyIcon::PopupIconMenu(HWND hWnd)
{
    NOTIFYICONIDENTIFIER icon_id = { sizeof(NOTIFYICONIDENTIFIER) };
    icon_id.guidItem = guid;
    RECT notify_rect;
    if(FAILED(Shell_NotifyIconGetRect(&icon_id, &notify_rect)))
        return;

    TPMPARAMS tpmp = { sizeof(TPMPARAMS) };
    tpmp.rcExclude = notify_rect;
    TrackPopupMenuEx(GetSubMenu(popup_menu, 0), 0, notify_rect.left, notify_rect.top, hWnd, &tpmp);
}

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

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
    LoadStringW(hInstance, IDC_HDRTRAY, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HDRTRAY));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
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
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HDRTRAY));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_HDRTRAY);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

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
        notify_icon->SetFromHDRStatus(hdr::GetWindowsHDRStatus());
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DISPLAYCHANGE:
        notify_icon->SetFromHDRStatus(hdr::GetWindowsHDRStatus());
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

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
