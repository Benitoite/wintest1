#include <gtk/gtk.h>
#include <windows.h>

// Track current mode
bool current_dark_mode = false;

// Check Windows dark mode via registry
bool isWindowsDarkTheme() {
    HKEY hKey;
    DWORD value = 1;
    DWORD size = sizeof(DWORD);
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        RegQueryValueExW(hKey, L"AppsUseLightTheme", NULL, NULL, (LPBYTE)&value, &size);
        RegCloseKey(hKey);
    }
    return value == 0; // 0 = dark mode
}

// Timer callback: re-check registry and apply theme if needed
gboolean poll_dark_mode(gpointer data) {
    bool new_dark_mode = isWindowsDarkTheme();
    if (new_dark_mode != current_dark_mode) {
        current_dark_mode = new_dark_mode;

        const char* theme = new_dark_mode ? "Adwaita-dark" : "Adwaita";
        GtkSettings* settings = gtk_settings_get_default();
        g_object_set(settings, "gtk-theme-name", theme, NULL);

        gtk_widget_queue_draw(GTK_WIDGET(data));
        g_print("Switched to %s mode\n", theme);
    }

    return G_SOURCE_CONTINUE;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int argc = 0;
    char** argv = NULL;
    gtk_init(&argc, &argv);

    // Create main window
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    gtk_window_set_title(GTK_WINDOW(window), "GTK Dark Mode Polling");

    // Initial theme
    current_dark_mode = isWindowsDarkTheme();
    const char* theme = current_dark_mode ? "Adwaita-dark" : "Adwaita";
    GtkSettings* settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-theme-name", theme, NULL);

    // Poll every 3 seconds
    g_timeout_add_seconds(3, poll_dark_mode, window);

    // Close event
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
