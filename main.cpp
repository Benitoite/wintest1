#include <gtk/gtk.h>
#include <windows.h>
#include <cstdlib>

bool current_dark_mode = false;
GtkWidget* window = nullptr;

// Get current Windows light/dark app mode
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

// Fade out and destroy current window
void destroy_window_with_fade() {
    if (!window) return;

    gtk_widget_set_opacity(window, 1.0);
    for (double i = 1.0; i >= 0.0; i -= 0.1) {
        gtk_widget_set_opacity(window, i);
        while (gtk_events_pending()) gtk_main_iteration();
        g_usleep(30 * 1000); // 30ms
    }

    gtk_widget_destroy(window);
    window = nullptr;
}

// Fade in window
void fade_in_window(GtkWidget* win) {
    gtk_widget_set_opacity(win, 0.0);
    while (gtk_events_pending()) gtk_main_iteration();
    for (double i = 0.0; i <= 1.0; i += 0.1) {
        gtk_widget_set_opacity(win, i);
        while (gtk_events_pending()) gtk_main_iteration();
        g_usleep(30 * 1000); // 30ms
    }
}

// Build the main window
void build_main_window() {
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    gtk_window_set_title(GTK_WINDOW(window), "GTK Theme Switch");

    GtkWidget* textview = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD_CHAR);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview)),
        "This window now fades when switching themes.\n"
        "Try changing system light/dark mode!",
        -1);

    gtk_container_add(GTK_CONTAINER(window), textview);

    // Only quit app when user closes window (not when we destroy/rebuild it)
    g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    fade_in_window(window);
}

// Poll every 3 seconds and rebuild window on dark mode change
gboolean poll_dark_mode(gpointer data) {
    bool new_dark_mode = isWindowsDarkTheme();
    if (new_dark_mode != current_dark_mode) {
        current_dark_mode = new_dark_mode;
        const char* theme = current_dark_mode ? "Adwaita-dark" : "Adwaita";
        g_setenv("GTK_THEME", theme, TRUE);
        GtkSettings* settings = gtk_settings_get_default();
        g_object_set(settings, "gtk-theme-name", theme, NULL);

        g_print("Theme change detected: switching to %s\n", theme);

        destroy_window_with_fade();
        build_main_window();
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
