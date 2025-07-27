#include <gtk/gtk.h>
#include <windows.h>
#include <thread>
#include <chrono>

bool isDarkModeActive() {
    HKEY hKey;
    DWORD value = 1;
    DWORD size = sizeof(DWORD);

    if (RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr, (LPBYTE)&value, &size);
        RegCloseKey(hKey);
    }

    return value == 0; // 0 = dark mode
}

void applyTheme() {
    GtkSettings* settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-application-prefer-dark-theme", isDarkModeActive(), nullptr);
}

void restartApp() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWNORMAL);
    exit(0);
}

void monitorThemeChanges(bool initialDarkMode) {
    std::thread([initialDarkMode]() {
        bool lastMode = initialDarkMode;
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            bool currentMode = isDarkModeActive();
            if (currentMode != lastMode) {
                // Theme changed → fade + restart
                PostMessageA(GetForegroundWindow(), WM_CLOSE, 0, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(600));
                restartApp();
                break;
            }
        }
    }).detach();
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    gtk_init(nullptr, nullptr);

    applyTheme();                          // Set theme before window

    bool darkMode = isDarkModeActive();
    monitorThemeChanges(darkMode);        // Background polling

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
    gtk_window_set_title(GTK_WINDOW(window), darkMode ? "Dark Mode" : "Light Mode");

    GtkWidget* label = gtk_label_new(darkMode ? "Dark mode is active" : "Light mode is active");
    gtk_container_add(GTK_CONTAINER(window), label);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
