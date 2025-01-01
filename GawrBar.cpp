#include "framework.h"
#include "GawrBar.h"
#include <dwmapi.h>                 
#include <cmath>                    
#include <windows.h>
#include <string>
#include <set>
#include <tchar.h>
#include <iostream>
#include <shellapi.h>               
#include <shlwapi.h>                
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "dwmapi.lib")

#define MAX_LOADSTRING 100
#define TIMER_ID 1
#define TIMER_INTERVAL 500 

#define IDR_YURUKA 101

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING], szWindowClass[MAX_LOADSTRING];

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int screenWidth;
int screenHeight;
int secWidth;

enum TaskbarMode { MODE_DOCKLEFT, MODE_DOCKCENTER, MODE_SPLIT, MODE_CENTER };
TaskbarMode currentMode = MODE_SPLIT;

int appCount = 0;
NOTIFYICONDATA nid;

ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDC_GAWRBAR);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

    return RegisterClassExW(&wcex);
}

BOOL AddCustomFont() {

    HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(IDR_YURUKA), RT_FONT);
    if (hRes == NULL) {
        return FALSE;
    }

    HGLOBAL hData = LoadResource(hInst, hRes);
    if (hData == NULL) {
        return FALSE;
    }

    void* pData = LockResource(hData);
    if (pData == NULL) {
        return FALSE;
    }

    HANDLE hFont = AddFontMemResourceEx(pData, SizeofResource(hInst, hRes), NULL, NULL);
    return hFont != NULL;
}

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

int GetAppCount() {
    appCount = 0;
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {

        if (IsWindowVisible(hwnd) && !GetWindow(hwnd, GW_OWNER)) {
            wchar_t windowTitle[1024];
            GetWindowText(hwnd, windowTitle, sizeof(windowTitle) / sizeof(wchar_t));

            if (wcslen(windowTitle) > 0) {

                if (wcscmp(windowTitle, L"Program Manager") == 0) {
                    return TRUE;
                }
                if (wcscmp(windowTitle, L"Explorer") == 0) {
                    return FALSE;
                }

                wchar_t className[256];
                GetClassName(hwnd, className, sizeof(className) / sizeof(wchar_t));

                if (wcscmp(className, L"Shell_TrayWnd") == 0) {

                    return FALSE;
                }

                if (wcscmp(className, L"Button") == 0 || wcscmp(className, L"SysListView32") == 0) {

                    return TRUE;
                }

                appCount++;
            }
        }

        return TRUE;
        }, 0);

    return appCount;
}

bool IsTaskbarWindow(HWND hwnd) {
    if (!IsWindowVisible(hwnd)) {
        return false;
    }

    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

    if (exStyle & WS_EX_TOOLWINDOW) {
        return false;
    }

    if (exStyle & WS_EX_APPWINDOW) {
        return true;
    }

    HWND owner = GetWindow(hwnd, GW_OWNER);
    if (owner != nullptr) {
        return false;
    }

    return true;
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

void ApplyTaskbarMode() {
    HWND hTaskbar = FindWindow(L"Shell_TrayWnd", NULL);
    if (!hTaskbar) {
        OutputDebugString(L"[Error] Taskbar not found.\n");
        return;
    }

    secWidth = GetAppCount() * 80;

    if (currentMode == MODE_SPLIT) {
        int margin = 4;
        int sectionHeight = 86;
        SetTaskbarAlignment(false);
        HRGN hRgn1 = CreateRoundRectRgn(margin, margin, secWidth + 180, sectionHeight - margin - 1, 20, 20);
        HRGN hRgn2 = CreateRoundRectRgn(screenWidth * 0.75, margin, screenWidth - margin, sectionHeight - margin - 1, 20, 20);

        HRGN hCombinedRgn = CreateRectRgn(0, 0, 0, 0);
        CombineRgn(hCombinedRgn, hRgn1, hRgn2, RGN_OR);

        SetWindowRgn(hTaskbar, hCombinedRgn, TRUE);

        DeleteObject(hRgn1);
        DeleteObject(hRgn2);
    }
    else if (currentMode == MODE_DOCKLEFT) {

        int leftMargin = 4, rightMargin = 4, upperMargin = 4, lowerMargin = 4;
        SetTaskbarAlignment(false);
        HRGN hRgn = CreateRoundRectRgn(leftMargin, upperMargin, screenWidth - rightMargin, 86 - lowerMargin - 1, 20, 20);
        SetWindowRgn(hTaskbar, hRgn, TRUE);
        DeleteObject(hRgn);
    }
    else if (currentMode == MODE_DOCKCENTER) {

        int leftMargin = 4, rightMargin = 4, upperMargin = 4, lowerMargin = 4;
        SetTaskbarAlignment(true);
        HRGN hRgn = CreateRoundRectRgn(leftMargin, upperMargin, screenWidth - rightMargin, 86 - lowerMargin - 1, 20, 20);
        SetWindowRgn(hTaskbar, hRgn, TRUE);
        DeleteObject(hRgn);
    }
    else if (currentMode == MODE_CENTER) {

        SetTaskbarAlignment(true);
        int sectionHeight = 86;
        int startX = (screenWidth - secWidth) / 2;
        int margin = 4;

        HRGN hRgn1 = CreateRoundRectRgn(startX - 80, margin, startX + secWidth + 80, sectionHeight - margin - 1, 20, 20);
        HRGN hRgn2 = CreateRoundRectRgn(screenWidth * 0.75, margin, screenWidth - margin, sectionHeight - margin - 1, 20, 20);

        HRGN hCombinedRgn = CreateRectRgn(0, 0, 0, 0);
        CombineRgn(hCombinedRgn, hRgn1, hRgn2, RGN_OR);
        SetWindowRgn(hTaskbar, hCombinedRgn, TRUE);
        DeleteObject(hRgn1);
        DeleteObject(hRgn2);
    }

    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hTaskbar, &margins);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance;
    HWND hWnd = CreateWindowExW(WS_EX_TOOLWINDOW, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 100, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) return FALSE;

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

    if (!Shell_NotifyIcon(NIM_ADD, &nid)) {
        OutputDebugString(L"[Error] Failed to add system tray icon.\n");
    }

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static int lastAppCount = 0;

    switch (message) {
    case WM_CREATE:
        SetTimer(hWnd, TIMER_ID, TIMER_INTERVAL, NULL);
        break;

    case WM_TIMER:
        if (wParam == TIMER_ID) {
            int currentAppCount = GetAppCount();
            if (currentAppCount != lastAppCount) {
                lastAppCount = currentAppCount;
                ApplyTaskbarMode();
                OutputDebugString(L"[Debug] Updated.\n");
            }
        }
        break;

    case WM_USER + 1:
        if (lParam == WM_LBUTTONDOWN) {
            if (IsWindowVisible(hWnd))
                ShowWindow(hWnd, SW_HIDE);
            else
                ShowWindow(hWnd, SW_SHOW);
        }
        if (lParam == WM_RBUTTONDOWN) {
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();
            if (hMenu) {
                AppendMenu(hMenu, MF_STRING, IDM_SHOW, L"Control Panel");
                HMENU hSubMenu = CreatePopupMenu();
                HMENU hAlignMenu = CreatePopupMenu();
                AppendMenu(hAlignMenu, MF_STRING | (currentMode == MODE_DOCKLEFT ? MF_CHECKED : 0), IDM_MODE_DOCKLEFT, L"Left");
                AppendMenu(hAlignMenu, MF_STRING | (currentMode == MODE_DOCKCENTER ? MF_CHECKED : 0), IDM_MODE_DOCKCENTER, L"Center");
                AppendMenu(hSubMenu, MF_POPUP, (UINT_PTR)hAlignMenu, L"Shoreline");
                AppendMenu(hSubMenu, MF_STRING | (currentMode == MODE_CENTER ? MF_CHECKED : 0), IDM_MODE_CENTER, L"Core");
                AppendMenu(hSubMenu, MF_STRING | (currentMode == MODE_SPLIT ? MF_CHECKED : 0), IDM_MODE_SPLIT, L"Tide");
                AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"Configure Layout");
                AppendMenu(hMenu, MF_STRING, IDM_ABOUT, L"About");
                AppendMenu(hMenu, MF_STRING, IDM_EXIT, L"Exit");

                SetForegroundWindow(hWnd);
                TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x - 10, pt.y - 10, 0, hWnd, NULL);
                DestroyMenu(hMenu);
            }
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDM_SHOW:
            ShowWindow(hWnd, SW_SHOW);
            SetForegroundWindow(hWnd);
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

        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;

        case IDM_EXIT:

            Shell_NotifyIcon(NIM_DELETE, &nid);

            DestroyWindow(hWnd);
            PostQuitMessage(0);
            break;

            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rect;
        GetClientRect(hWnd, &rect);
        int padding = 8;

        HBRUSH hBrush = CreateSolidBrush(RGB(51, 115, 157));
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);

        HFONT hFont = CreateFont(
            24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Yuruka.otf");
        SelectObject(hdc, hFont);

        WCHAR text[100];

        SetTextColor(hdc, RGB(190, 68, 73));
        wsprintf(text, L"GawrBar Taskbar Customisation Utility");
        TextOut(hdc, padding, padding, text, lstrlen(text));

        SetTextColor(hdc, RGB(156, 201, 217));
        wsprintf(text, L"Screen Width: %d px", screenWidth);
        TextOut(hdc, padding, padding + 40, text, lstrlen(text));

        wsprintf(text, L"You are running Version 1.0");
        TextOut(hdc, padding, padding + 80, text, lstrlen(text));

        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(147, 169, 201));
        SelectObject(hdc, hPen);
        MoveToEx(hdc, padding, padding + 160, NULL);
        LineTo(hdc, rect.right - padding, padding + 160);

        DeleteObject(hPen);
        DeleteObject(hFont);

        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
    {
        HWND hTaskbar = FindWindow(L"Shell_TrayWnd", NULL);
        KillTimer(hWnd, TIMER_ID);
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
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