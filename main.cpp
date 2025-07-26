
#include <windows.h>
#include <dwmapi.h>
#include <gtk/gtk.h>
#include <gdk/gdkwin32.h>
#include <uxtheme.h>
#include <string>

// Function to apply the GTK theme based on Windows light/dark mode
void apply_gtk_theme_based_on_windows_mode() {
    HKEY hKey;
    DWORD value = 1;
    DWORD dataSize = sizeof(DWORD);

    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &dataSize);
        RegCloseKey(hKey);
    }

    if (value == 0) {
        GtkSettings *settings = gtk_settings_get_default();
        g_object_set(settings, "gtk-theme-name", "Adwaita-dark", NULL);
    } else {
        GtkSettings *settings = gtk_settings_get_default();
        g_object_set(settings, "gtk-theme-name", "Adwaita", NULL);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"MyNativeWindowClass";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"My GTK App",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, hInstance, nullptr
    );

    gtk_init(nullptr, nullptr);
    apply_gtk_theme_based_on_windows_mode();

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GTK in Native Window");
    gtk_widget_set_size_request(window, 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_realize(window);
    HWND gtkHWND = (HWND)gdk_win32_window_get_handle(gtk_widget_get_window(window));
    SetParent(gtkHWND, hwnd);

    ShowWindow(hwnd, nCmdShow);
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
