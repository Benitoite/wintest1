#include <gtk/gtk.h>
#include <windows.h>
#include <cstdlib>

bool current_dark_mode = false;
GtkWidget* window = nullptr;

// Check Windows dark mode setting
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
    return value == 0;
}

// Build the window with current theme
void build_main_window() {
    if (window) {
        gtk_widget_destroy(window);
    }

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    gtk_window_set_title(GTK_WINDOW(window), "GTK Theme Switch");

    GtkWidget* textview = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD_CHAR);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview)),
        "This window restarts itself when you toggle light/dark mode in Windows.",
        -1);

    gtk_container_add(GTK_CONTAINER(window), textview);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);
}

// Poll for dark mode changes and rebuild window if needed
gboolean poll_dark_mode(gpointer data) {
    bool new_dark_mode = isWindowsDarkTheme();
    if (new_dark_mode != current_dark_mode) {
        current_dark_mode = new_dark_mode;
        const char* theme = current_dark_mode ? "Adwaita-dark" : "Adwaita";
        g_setenv("GTK_THEME", theme, TRUE);

        GtkSettings* settings = gtk_settings_get_default();
        g_object_set(settings, "gtk-theme-name", theme, NULL);
        g_print("Switching to %s mode...\n", theme);

        build_main_window();  // Rebuild window to reflect new theme
    }
    return G_SOURCE_CONTINUE;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int argc = 0;
    char** argv = NULL;

    current_dark_mode = isWindowsDarkTheme();
    const char* theme = current_dark_mode ? "Adwaita-dark" : "Adwaita";
    g_setenv("GTK_THEME", theme, TRUE);

    gtk_init(&argc, &argv);

    GtkSettings* settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-theme-name", theme, NULL);

    build_main_window();
    g_timeout_add_seconds(3, poll_dark_mode, NULL);
    gtk_main();
    return 0;
}
