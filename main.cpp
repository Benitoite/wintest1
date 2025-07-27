#include <gtk/gtk.h>
#include <windows.h>
#include <dwmapi.h>
#include <thread>
#include <chrono>
#include <string>
#ifdef _WIN32
#include <gdk/gdkwin32.h>
#endif
#pragma comment(lib, "dwmapi.lib")

bool is_dark_mode_enabled() {
    HKEY hKey;
    DWORD value = 1;
    DWORD size = sizeof(DWORD);
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &size);
        RegCloseKey(hKey);
    }
    return value == 0;
}

void set_dark_mode_titlebar(HWND hwnd, bool dark) {
    if (!hwnd) return;
    BOOL useDark = dark ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, 20, &useDark, sizeof(useDark));
}

GtkWidget *window;
bool current_dark_mode = false;

void monitor_dark_mode() {
    while (true) {
        bool dark = is_dark_mode_enabled();
        if (dark != current_dark_mode) {
            current_dark_mode = dark;

            gdk_threads_add_idle([](void*) -> gboolean {
                HWND hwnd = (HWND)gdk_win32_window_get_handle(gtk_widget_get_window(window));
                set_dark_mode_titlebar(hwnd, current_dark_mode);

                gtk_style_context_reset_widgets(gtk_widget_get_screen(window));
                return G_SOURCE_REMOVE;
            }, nullptr);
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    gtk_init(nullptr, nullptr);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Dark Mode Test");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_widget_show_all(window);

    current_dark_mode = is_dark_mode_enabled();
    HWND hwnd = (HWND)gdk_win32_window_get_handle(gtk_widget_get_window(window));
    set_dark_mode_titlebar(hwnd, current_dark_mode);

    std::thread dark_mode_thread(monitor_dark_mode);
    dark_mode_thread.detach();

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);
    gtk_main();
    return 0;
}
