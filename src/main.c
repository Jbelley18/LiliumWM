#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <math.h>

// Global variables
const wchar_t CLASS_NAME[] = L"Sample Window Class";

// Define layout types
typedef enum {
    VERTICAL_TILING,
    HORIZONTAL_TILING,
    GRID_LAYOUT
} LayoutType;

LayoutType currentLayout = VERTICAL_TILING;  // Start with vertical tiling

// Monitor information
typedef struct {
    HMONITOR hMonitor;
    RECT rcMonitor;
} MONITORINFOEX;

MONITORINFOEX monitors[10];
int monitorCount = 0;

// Forward declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
void ApplyLayout(LayoutType layout);
void ApplyVerticalTiling(HWND* windows, int count);
void ApplyHorizontalTiling(HWND* windows, int count);
void ApplyGridLayout(HWND* windows, int count);

// Main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc)) {
        return 1;  // Fail if class registration fails
    }

    HWND hwnd = CreateWindowEx(
        0,                          // Optional styles
        CLASS_NAME,                 // Window class name
        L"Learn to Program Windows",// Window title
        WS_OVERLAPPEDWINDOW,        // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,  // Size and position defaults
        NULL,                       // Parent window    
        NULL,                       // Menu
        hInstance,                  // Instance handle
        NULL                        // Additional application data
    );

    if (hwnd == NULL) {
        return 0;  // Exit if window creation fails
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Enumerate all monitors
    monitorCount = 0;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);

    // Apply the initial layout
    ApplyLayout(currentLayout);

    // Register hotkeys
    RegisterHotKey(NULL, 1, MOD_ALT, 'V');  // Alt + V for Vertical Tiling
    RegisterHotKey(NULL, 2, MOD_ALT, 'H');  // Alt + H for Horizontal Tiling
    RegisterHotKey(NULL, 3, MOD_ALT, 'G');  // Alt + G for Grid Layout

    // Standard message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_HOTKEY) {
            switch (msg.wParam) {
                case 1:  // Vertical Tiling
                    currentLayout = VERTICAL_TILING;
                    ApplyLayout(currentLayout);
                    break;
                case 2:  // Horizontal Tiling
                    currentLayout = HORIZONTAL_TILING;
                    ApplyLayout(currentLayout);
                    break;
                case 3:  // Grid Layout
                    currentLayout = GRID_LAYOUT;
                    ApplyLayout(currentLayout);
                    break;
            }
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    monitors[monitorCount].hMonitor = hMonitor;
    monitors[monitorCount].rcMonitor = *lprcMonitor;
    monitorCount++;
    return TRUE;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    static HWND windows[256];
    static int count = 0;

    if (IsWindowVisible(hwnd)) {
        windows[count++] = hwnd;
    }

    if (count == 256) {
        return FALSE;  // Avoid overflow
    }

    if (count > 0 && hwnd == NULL) {  // If we reached the end
        LayoutType layout = (LayoutType)lParam;
        if (layout == VERTICAL_TILING) {
            ApplyVerticalTiling(windows, count);
        } else if (layout == HORIZONTAL_TILING) {
            ApplyHorizontalTiling(windows, count);
        } else if (layout == GRID_LAYOUT) {
            ApplyGridLayout(windows, count);
        }
        count = 0;  // Reset count for next enumeration
    }

    return TRUE;
}

void ApplyLayout(LayoutType layout) {
    EnumWindows(EnumWindowsProc, (LPARAM)layout);
}

void ApplyVerticalTiling(HWND* windows, int count) {
    for (int m = 0; m < monitorCount; m++) {
        int monitorHeight = monitors[m].rcMonitor.bottom - monitors[m].rcMonitor.top;
        int monitorWidth = monitors[m].rcMonitor.right - monitors[m].rcMonitor.left;
        int windowHeight = monitorHeight / count;

        for (int i = 0; i < count; i++) {
            SetWindowPos(windows[i], HWND_TOP, monitors[m].rcMonitor.left, monitors[m].rcMonitor.top + i * windowHeight,
                monitorWidth, windowHeight, SWP_NOZORDER);
        }
    }
}

void ApplyHorizontalTiling(HWND* windows, int count) {
    for (int m = 0; m < monitorCount; m++) {
        int monitorHeight = monitors[m].rcMonitor.bottom - monitors[m].rcMonitor.top;
        int monitorWidth = monitors[m].rcMonitor.right - monitors[m].rcMonitor.left;
        int windowWidth = monitorWidth / count;

        for (int i = 0; i < count; i++) {
            SetWindowPos(windows[i], HWND_TOP, monitors[m].rcMonitor.left + i * windowWidth, monitors[m].rcMonitor.top,
                windowWidth, monitorHeight, SWP_NOZORDER);
        }
    }
}

void ApplyGridLayout(HWND* windows, int count) {
    for (int m = 0; m < monitorCount; m++) {
        int monitorHeight = monitors[m].rcMonitor.bottom - monitors[m].rcMonitor.top;
        int monitorWidth = monitors[m].rcMonitor.right - monitors[m].rcMonitor.left;
        int rows = (int)sqrt(count);
        int cols = (count + rows - 1) / rows;  // Ceiling of count / rows

        int windowWidth = monitorWidth / cols;
        int windowHeight = monitorHeight / rows;

        for (int i = 0; i < count; i++) {
            int row = i / cols;
            int col = i % cols;
            SetWindowPos(windows[i], HWND_TOP, monitors[m].rcMonitor.left + col * windowWidth, monitors[m].rcMonitor.top + row * windowHeight,
                windowWidth, windowHeight, SWP_NOZORDER);
        }
    }
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
            FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1)); // Fill the background

            DrawText(hdc, L"Hello, World!", -1, &rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_SIZE: {
            // Invalidate the window to trigger a repaint
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}
