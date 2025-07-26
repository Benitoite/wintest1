#include <gtk/gtk.h>
#include <windows.h>
#include <cstdlib>

bool current_dark_mode = false;

// Detect if Windows is in dark mode for apps
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

// Force a GTK theme reload for all widgets
void force_theme_reload(GtkWidget* widget) {
    GtkStyleContext* context = gtk_widget_get_style_context(widget);
    gtk_style_context_invalidate(context);
    gtk_widget_queue_draw(widget);

    if (GTK_IS_CONTAINER(widget)) {
        gtk_container_foreach(GTK_CONTAINER(widget),
            (GtkCallback)force_theme_reload,
            NULL);
    }
}

// Poll and apply dark mode change
gboolean poll_dark_mode(gpointer data) {
    bool new_dark_mode = isWindowsDarkTheme();
    if (new_dark_mode != current_dark_mode) {
        current_dark_mode = new_dark_mode;
        const char* theme = new_dark_mode ? "Adwaita-dark" : "Adwaita";

        g_setenv("GTK_THEME", theme, TRUE);
        GtkSettings* settings = gtk_settings_get_default();
        g_object_set(settings, "gtk-theme-name", theme, NULL);

        GtkWidget* window = GTK_WIDGET(data);
        force_theme_reload(window);
        gtk_widget_queue_draw(window);
        g_print("Switched to %s mode\n", theme);
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

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    gtk_window_set_title(GTK_WINDOW(window), "GTK Dark Mode Test");

    GtkSettings* settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-theme-name", theme, NULL);

    // Add a widget that will visibly reflect the theme
    GtkWidget* textview = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD_CHAR);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview)),
        "This widget should visibly change when switching light/dark mode in Windows.",
        -1);

    gtk_container_add(GTK_CONTAINER(window), textview);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);

    // Periodically check for theme changes
    g_timeout_add_seconds(3, poll_dark_mode, window);

    gtk_main();
    return 0;
}
