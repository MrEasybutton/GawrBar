#include "framework.h"
#include "GawrBar.h"
#include <dwmapi.h>                 // Window Framing API
#include <cmath>                    // Math stuff 
#include <windows.h>                
#include <shellapi.h>               // System Tray Icons
#pragma comment(lib, "dwmapi.lib")

#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING], szWindowClass[MAX_LOADSTRING];

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int screenWidth; // Screen Width (this currently has issues so a fix is issued below)
int screenHeight;

NOTIFYICONDATA nid;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GAWRBAR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GAWRBAR));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance,
                         LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAWRBAR)), LoadCursor(nullptr, IDC_ARROW),
                         (HBRUSH)(COLOR_WINDOW + 1), MAKEINTRESOURCEW(IDC_GAWRBAR), szWindowClass,
                         LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL)) };
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    HWND hWnd = CreateWindowExW(WS_EX_TOOLWINDOW, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 100, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
        return FALSE;

    int sysWidthRead = GetSystemMetrics(SM_CXSCREEN); // This is inaccurate apparently
    screenWidth = round(sysWidthRead + round((sysWidthRead - 4) * 0.754));
    screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int leftMargin = 4, rightMargin = 4, upperMargin = 4, lowerMargin = 4;
    int windowWidth = screenWidth - leftMargin - rightMargin, windowHeight = 200;

    SetWindowPos(hWnd, HWND_TOP, 0, 20, 500, 560, SWP_NOZORDER);

    HWND hTaskbar = FindWindow(L"Shell_TrayWnd", NULL);
    if (hTaskbar) {
        HRGN hRgn = CreateRoundRectRgn(leftMargin, upperMargin, screenWidth - rightMargin, 86 - lowerMargin, 20, 20);
        SetWindowRgn(hTaskbar, hRgn, TRUE);
        MARGINS margins = { -1 };
        DwmExtendFrameIntoClientArea(hTaskbar, &margins);
    }
    else {
        MessageBox(NULL, L"Taskbar not found!", L"Error", MB_ICONERROR);
        return FALSE;
    }

    ShowWindow(hWnd, SW_HIDE);
    UpdateWindow(hWnd);

    memset(&nid, 0, sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;
    nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAWRBAR));
    wcscpy_s(nid.szTip, L"GawrBar"); // System Tray Icon

    Shell_NotifyIcon(NIM_ADD, &nid);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_SHOW:
            ShowWindow(hWnd, SW_SHOW);
            SetForegroundWindow(hWnd);
            break;
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

    case WM_USER + 1:
    {
        switch (lParam)
        {
        case WM_LBUTTONDOWN:
            if (IsWindowVisible(hWnd))
                ShowWindow(hWnd, SW_HIDE);
            else
                ShowWindow(hWnd, SW_SHOW);
            break;
        
        case WM_RBUTTONDOWN:
        {
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();
            if (hMenu)
            {
                AppendMenu(hMenu, MF_STRING, IDM_SHOW, L"Control Panel");
                AppendMenu(hMenu, MF_STRING, IDM_ABOUT, L"About");
                AppendMenu(hMenu, MF_STRING, IDM_EXIT, L"Exit");

                SetForegroundWindow(hWnd);
                TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x - 10, pt.y - 10, 0, hWnd, NULL);
                DestroyMenu(hMenu);
            }
        }
        break;
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Temp Info for debug
        WCHAR text[50];
        wsprintf(text, L"Your screen width seems to be: %d", screenWidth);
        TextOut(hdc, 10, 10, text, lstrlen(text));

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
    {
        HWND hTaskbar = FindWindow(L"Shell_TrayWnd", NULL);
        if (hTaskbar)
        {
            SetWindowRgn(hTaskbar, NULL, TRUE);

            MARGINS margins = { 0 };
            DwmExtendFrameIntoClientArea(hTaskbar, &margins);
        }

        PostQuitMessage(0);
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_COMMAND && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)) {
        EndDialog(hDlg, LOWORD(wParam));
        return TRUE;
    }
    return FALSE;
}
