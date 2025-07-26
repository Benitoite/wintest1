// Compile with: g++ main.cpp -o gtk_native_window.exe `pkg-config --cflags --libs gtk+-3.0` -mwindows -lole32 -luxtheme -ldwmapi
#include <windows.h>
#include <dwmapi.h>
#include <gtk/gtk.h>
#include <gdk/gdkwin32.h>  // <-- This is the key missing header
#include <uxtheme.h>
#include <string>

// Globals
HINSTANCE hInstance;
GtkWidget* gtk_main_window;
bool is_dark_mode = false;

// Forward declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void apply_gtk_theme(bool dark);
bool detect_dark_mode();

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    hInstance = hInst;

    // Initialize GTK
    int argc = 0;
    char **argv = NULL;
    gtk_init(&argc, &argv);

    // Detect and apply theme
    is_dark_mode = detect_dark_mode();
    apply_gtk_theme(is_dark_mode);

    // Create GTK window
    gtk_main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gtk_main_window), "GTK Inside Native Window (Dark Mode Aware)");
    gtk_window_set_default_size(GTK_WINDOW(gtk_main_window), 300, 200);

    // Create the Win32 host window
    const wchar_t CLASS_NAME[] = L"MyNativeWindowClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"Native Win32 Wrapper",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, SW_SHOWDEFAULT);

    // Embed GTK window into HWND
    gtk_widget_realize(gtk_main_window);
    HWND gtkHWND = (HWND)gdk_win32_window_get_handle(gtk_widget_get_window(gtk_main_window));
    SetParent(gtkHWND, hwnd);
    SetWindowLongPtr(gtkHWND, GWL_STYLE, WS_CHILD | WS_VISIBLE);

    gtk_widget_show_all(gtk_main_window);

    // Message loop with GTK integration
    MSG msg = {};
    while (gtk_events_pending()) gtk_main_iteration();

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        while (gtk_events_pending()) gtk_main_iteration();
    }

    return 0;
}

bool detect_dark_mode() {
    HKEY hKey;
    DWORD dwType = REG_DWORD;
    DWORD dwData = 0;
    DWORD dwSize = sizeof(dwData);

    // Registry key for Windows 10/11 app mode setting
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, &dwType, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return dwData == 0; // 0 means dark mode
        }
        RegCloseKey(hKey);
    }
    return false; // default to light
}

void apply_gtk_theme(bool dark) {
    const char* theme = dark ? "Adwaita-dark" : "Adwaita";
    gtk_settings_set_string_property(
        gtk_settings_get_default(),
        "gtk-theme-name",
        theme,
        NULL
    );
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            gtk_main_quit();
            PostQuitMessage(0);
            return 0;

        case WM_SETTINGCHANGE:
        case WM_THEMECHANGED: {
            bool new_mode = detect_dark_mode();
            if (new_mode != is_dark_mode) {
                is_dark_mode = new_mode;
                apply_gtk_theme(is_dark_mode);
                gtk_widget_queue_draw(gtk_main_window);
            }
            break;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
