#include <gtk/gtk.h>
#include "ss_style.h"
#include "ss_controller.h"


void
set_image_scaled(SsWallpaperContext *ctx)
{
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(ctx->wallpaper_path,
                        ctx->wallpaper_geometry.width, ctx->wallpaper_geometry.height, TRUE, NULL);

    if (pixbuf)
    {
        gtk_image_set_from_pixbuf(GTK_IMAGE(ctx->img), pixbuf);
        g_object_unref(pixbuf);
    }
    else
    {
        ss_send_notify(SS_ERROR_WARNING, "não foi possivel alterar o wallpaper");
    }
}


void
ss_update_style_widget(SsWidgetStyleContext *ctx)
{
    GtkStyleContext *gtk_ctx = gtk_widget_get_style_context(ctx->widget);
    if (ctx->_first_style != SS_STYLE_NONE)
    {
        gtk_style_context_remove_class(gtk_ctx, ss_style_to_string(ctx->_first_style));
    }

    // Adiciona nova classe
    gtk_style_context_add_class(gtk_ctx, ss_style_to_string(ctx->style));
    ctx->_first_style = ctx->style;
}


void
ss_apply_theme(SsTheme *theme)
{
    ss_update_style_widget(&theme->label);
    ss_update_style_widget(&theme->frame);
    set_image_scaled(&theme->wallpaper);
}


void
ss_init_style()
{
    static gchar *fmt[] = \
    {
        ".%s { color: #cc0000; } ",
        ".%s { color: black; } ",
        ".%s { color: #00aa00; } ",
        ".%s { background-color: #cccccc; } ",
        ".%s { background-color: white; }",
    };

    gint len_buf = 0;
    gchar buffer[2048] = {0};

    for (gint i = 0; i < SS_STYLE_COUNT; i++)
    {
        len_buf += sprintf(buffer+len_buf, fmt[i], ss_style_to_string(i));
    }
    
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, buffer, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
}
