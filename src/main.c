#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <math.h>

const wchar_t CLASS_NAME[] = L"Sample Window Class";

typedef enum {
    VERTICAL_TILING,
    HORIZONTAL_TILING,
    GRID_LAYOUT
} LayoutType;

LayoutType currentLayout = VERTICAL_TILING;  // Start with vertical tiling

typedef struct {
    MONITORINFOEX Monitor;
    RECT rcMonitor;
} MYMONITORINFOEX;

MYMONITORINFOEX monitors[10];
int monitorCount = 0;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
void ApplyLayout(LayoutType layout);
void ApplyVerticalTiling(HWND* windows, int count);
void ApplyHorizontalTiling(HWND* windows, int count);
void ApplyGridLayout(HWND* windows, int count);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = { 0 };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Learn to Program Windows",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // Enumerate monitors
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rect;
            GetClientRect(hwnd, &rect); // Get the dimensions of the window's client area
            FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW+1)); // Fill the background

            DrawText(hdc, L"Hello, World!", -1, &rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_SIZE: {
            // Invalidate the window to trigger a repaint
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case WM_KEYDOWN: {
            if (wParam == VK_MENU) {  // Alt key pressed
                if (GetKeyState(0x56) & 0x8000) {  // Alt + V
                    currentLayout = VERTICAL_TILING;
                    ApplyLayout(currentLayout);
                } else if (GetKeyState(0x48) & 0x8000) {  // Alt + H
                    currentLayout = HORIZONTAL_TILING;
                    ApplyLayout(currentLayout);
                } else if (GetKeyState(0x47) & 0x8000) {  // Alt + G
                    currentLayout = GRID_LAYOUT;
                    ApplyLayout(currentLayout);
                }
            }
            return 0;
        }
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    static int count = 0;  // Keep track of how many windows we've handled
    HWND* windows = (HWND*)lParam;

    // Filter out windows we don't want to handle
    if (!IsWindowVisible(hwnd) || GetWindowTextLength(hwnd) == 0) {
        return TRUE;  // Continue enumerating
    }

    // Add the window to our list
    windows[count++] = hwnd;

    return TRUE;  // Continue enumerating
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    if (monitorCount < 10) {
        MYMONITORINFOEX mi;
        mi.Monitor.cbSize = sizeof(MONITORINFOEX);
        GetMonitorInfo(hMonitor, &mi.Monitor);
        mi.rcMonitor = mi.Monitor.rcMonitor;

        monitors[monitorCount++] = mi;

        wprintf(L"Monitor %d: left=%d, top=%d, right=%d, bottom=%d\n", monitorCount,
            mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom);
    }
    return TRUE;
}

void ApplyLayout(LayoutType layout) {
    HWND windows[50];
    int windowCount = 0;

    // Enumerate all visible top-level windows
    EnumWindows(EnumWindowsProc, (LPARAM)windows);

    // Apply the specified layout to the windows
    switch (layout) {
        case VERTICAL_TILING:
            ApplyVerticalTiling(windows, windowCount);
            break;
        case HORIZONTAL_TILING:
            ApplyHorizontalTiling(windows, windowCount);
            break;
        case GRID_LAYOUT:
            ApplyGridLayout(windows, windowCount);
            break;
    }
}

void ApplyVerticalTiling(HWND* windows, int count) {
    int windowIndex = 0;
    int windowsPerMonitor = (count + monitorCount - 1) / monitorCount; // Ceiling of count / monitorCount

    for (int m = 0; m < monitorCount; m++) {
        int monitorHeight = monitors[m].rcMonitor.bottom - monitors[m].rcMonitor.top;
        int monitorWidth = monitors[m].rcMonitor.right - monitors[m].rcMonitor.left;
        int windowHeight = monitorHeight / windowsPerMonitor;

        for (int i = 0; i < windowsPerMonitor && windowIndex < count; i++, windowIndex++) {
            SetWindowPos(windows[windowIndex], HWND_TOP, monitors[m].rcMonitor.left, monitors[m].rcMonitor.top + i * windowHeight,
                monitorWidth, windowHeight, SWP_NOZORDER);
            wprintf(L"Vertical Tiling: Monitor %d, Window %d: left=%d, top=%d, width=%d, height=%d\n", m + 1, windowIndex + 1,
                monitors[m].rcMonitor.left, monitors[m].rcMonitor.top + i * windowHeight, monitorWidth, windowHeight);
        }
    }
}

void ApplyHorizontalTiling(HWND* windows, int count) {
    int windowIndex = 0;
    int windowsPerMonitor = (count + monitorCount - 1) / monitorCount; // Ceiling of count / monitorCount

    for (int m = 0; m < monitorCount; m++) {
        int monitorHeight = monitors[m].rcMonitor.bottom - monitors[m].rcMonitor.top;
        int monitorWidth = monitors[m].rcMonitor.right - monitors[m].rcMonitor.left;
        int windowWidth = monitorWidth / windowsPerMonitor;

        for (int i = 0; i < windowsPerMonitor && windowIndex < count; i++, windowIndex++) {
            SetWindowPos(windows[windowIndex], HWND_TOP, monitors[m].rcMonitor.left + i * windowWidth, monitors[m].rcMonitor.top,
                windowWidth, monitorHeight, SWP_NOZORDER);
            wprintf(L"Horizontal Tiling: Monitor %d, Window %d: left=%d, top=%d, width=%d, height=%d\n", m + 1, windowIndex + 1,
                monitors[m].rcMonitor.left + i * windowWidth, monitors[m].rcMonitor.top, windowWidth, monitorHeight);
        }
    }
}

void ApplyGridLayout(HWND* windows, int count) {
    int windowIndex = 0;
    int windowsPerMonitor = (count + monitorCount - 1) / monitorCount; // Ceiling of count / monitorCount

    for (int m = 0; m < monitorCount; m++) {
        int monitorHeight = monitors[m].rcMonitor.bottom - monitors[m].rcMonitor.top;
        int monitorWidth = monitors[m].rcMonitor.right - monitors[m].rcMonitor.left;
        int rows = (int)sqrt(windowsPerMonitor);
        int cols = (windowsPerMonitor + rows - 1) / rows;  // Ceiling of windowsPerMonitor / rows

        int windowWidth = monitorWidth / cols;
        int windowHeight = monitorHeight / rows;

        for (int i = 0; i < windowsPerMonitor && windowIndex < count; i++, windowIndex++) {
            int row = i / cols;
            int col = i % cols;
            SetWindowPos(windows[windowIndex], HWND_TOP, monitors[m].rcMonitor.left + col * windowWidth, monitors[m].rcMonitor.top + row * windowHeight,
                windowWidth, windowHeight, SWP_NOZORDER);
            wprintf(L"Grid Layout: Monitor %d, Window %d: left=%d, top=%d, width=%d, height=%d\n", m + 1, windowIndex + 1,
                monitors[m].rcMonitor.left + col * windowWidth, monitors[m].rcMonitor.top + row * windowHeight, windowWidth, windowHeight);
        }
    }
}
