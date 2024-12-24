#include "framework.h"
#include "GawrBar.h"
#include <dwmapi.h>                 // Window Framing API
#include <cmath>                    // Math stuff 
#include <windows.h>   
#include <string>
#include <iostream>
#include <shellapi.h>               // System Tray Icons
#include <shlwapi.h>                // For registry hacks
#pragma comment(lib, "shlwapi.lib")
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


enum TaskbarMode { MODE_DOCKLEFT, MODE_DOCKCENTER, MODE_SPLIT, MODE_CENTER };
TaskbarMode currentMode = MODE_DOCKLEFT;


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


void SetTaskbarAlignment(bool center) {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",
        0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        DWORD value = center ? 1 : 0;
        RegSetValueEx(hKey, L"TaskbarAl", 0, REG_DWORD, (BYTE*)&value, sizeof(value));
        RegCloseKey(hKey);

        HWND hTaskbar = FindWindow(L"Shell_TrayWnd", NULL);
        PostMessage(hTaskbar, WM_SETTINGCHANGE, 0, (LPARAM)L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced");
    }
}

void ApplyTaskbarMode()
{
    HWND hTaskbar = FindWindow(L"Shell_TrayWnd", NULL);
    if (!hTaskbar) {
        MessageBox(NULL, L"Taskbar not found!", L"Error", MB_ICONERROR);
        return;
    }

    if (currentMode == MODE_DOCKLEFT) {
        // Dock Mode is simple, just applies a region with adjustable margins
        int leftMargin = 4, rightMargin = 4, upperMargin = 4, lowerMargin = 4;
        SetTaskbarAlignment(false);
        HRGN hRgn = CreateRoundRectRgn(leftMargin, upperMargin, screenWidth - rightMargin, 86 - lowerMargin - 1, 20, 20);
        SetWindowRgn(hTaskbar, hRgn, TRUE);
        DeleteObject(hRgn);
    }
    else if (currentMode == MODE_DOCKCENTER) {
        // Dock Mode is simple, just applies a region with adjustable margins
        int leftMargin = 4, rightMargin = 4, upperMargin = 4, lowerMargin = 4;
        SetTaskbarAlignment(true);
        HRGN hRgn = CreateRoundRectRgn(leftMargin, upperMargin, screenWidth - rightMargin, 86 - lowerMargin - 1, 20, 20);
        SetWindowRgn(hTaskbar, hRgn, TRUE);
        DeleteObject(hRgn);
    }
    else if (currentMode == MODE_SPLIT) {
        // Split Mode separates the taskbar into two regions.

        SetTaskbarAlignment(false);
        int totalSections = 3;
        int margin = 4;
        int sectionWidth = (screenWidth - (margin * (totalSections + 1))) / totalSections;
        int sectionHeight = 86;

        HRGN hRgn1 = CreateRoundRectRgn(margin, margin, margin + sectionWidth, sectionHeight - margin - 1, 20, 20);
        HRGN hRgn2 = CreateRoundRectRgn(screenWidth - sectionWidth + sectionHeight * 2, margin, screenWidth - margin, sectionHeight - margin - 1, 20, 20);

        HRGN hCombinedRgn = CreateRectRgn(0, 0, 0, 0);
        CombineRgn(hCombinedRgn, hRgn1, hRgn2, RGN_OR);

        SetWindowRgn(hTaskbar, hCombinedRgn, TRUE);

        DeleteObject(hRgn1);
        DeleteObject(hRgn2);
    }
    else if (currentMode == MODE_CENTER) {
        // Center Mode realigns the taskabr
        SetTaskbarAlignment(true);

        int sectionWidth = screenWidth / 3;
        int sectionHeight = 86;
        int startX = (screenWidth - sectionWidth) / 2;
        int margin = 4;

        HRGN hRgn1 = CreateRoundRectRgn(startX, margin, startX + sectionWidth, sectionHeight - margin - 1, 20, 20);
        HRGN hRgn2 = CreateRoundRectRgn(screenWidth - sectionWidth + sectionHeight * 2, margin, screenWidth - margin, sectionHeight - margin - 1, 20, 20);

        HRGN hCombinedRgn = CreateRectRgn(0, 0, 0, 0);
        CombineRgn(hCombinedRgn, hRgn1, hRgn2, RGN_OR);
        SetWindowRgn(hTaskbar, hCombinedRgn, TRUE);
        DeleteObject(hRgn1);
        DeleteObject(hRgn2);
    }


    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hTaskbar, &margins);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    HWND hWnd = CreateWindowExW(WS_EX_TOOLWINDOW, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 100, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
        return FALSE;

    int sysWidthRead = GetSystemMetrics(SM_CXSCREEN);
    screenWidth = round(sysWidthRead + round((sysWidthRead - 4) * 0.754));
    screenHeight = GetSystemMetrics(SM_CYSCREEN);

    ApplyTaskbarMode();

    ShowWindow(hWnd, SW_HIDE);
    UpdateWindow(hWnd);

    memset(&nid, 0, sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;
    nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAWRBAR));
    wcscpy_s(nid.szTip, L"GawrBar");

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

        case IDM_MODE_DOCKLEFT:
            currentMode = MODE_DOCKLEFT;
            ApplyTaskbarMode();
            break;
        case IDM_MODE_DOCKCENTER:
            currentMode = MODE_DOCKCENTER;
            ApplyTaskbarMode();
            break;
        case IDM_MODE_CENTER:
            currentMode = MODE_CENTER;
            ApplyTaskbarMode();
            break;
        case IDM_MODE_SPLIT:
            currentMode = MODE_SPLIT;
            ApplyTaskbarMode();
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
                HMENU hSubMenu = CreatePopupMenu();
                HMENU hAlignMenu = CreatePopupMenu();
                AppendMenu(hAlignMenu, MF_STRING | (currentMode == MODE_DOCKLEFT ? MF_CHECKED : 0), IDM_MODE_DOCKLEFT, L"Left");
                AppendMenu(hAlignMenu, MF_STRING | (currentMode == MODE_DOCKCENTER ? MF_CHECKED : 0), IDM_MODE_DOCKCENTER, L"Center");
                AppendMenu(hSubMenu, MF_POPUP, (UINT_PTR)hAlignMenu, L"Shoreline");
                AppendMenu(hSubMenu, MF_STRING | (currentMode == MODE_CENTER ? MF_CHECKED : 0), IDM_MODE_CENTER, L"Core");
                AppendMenu(hSubMenu, MF_STRING | (currentMode == MODE_SPLIT ? MF_CHECKED : 0), IDM_MODE_SPLIT, L"Tide");
                AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"Configure Beach");
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
