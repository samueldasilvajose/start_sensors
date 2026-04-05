#include <gtk/gtk.h>

#include "ss_header.h"
#include "ss_ui_error.h"
#include "ss_header_settings.h"
#include "../core/ss_yaml_types.h"
#include "../controller/ss_controller.h"


SsHSettingsContext *
ss_hsettings_context_new()
{
    SsHSettingsContext *ctx = g_malloc0(sizeof(SsHSettingsContext));
    if (!ctx)
    {
        ss_send_notify(SS_ERROR_FATAL, "não foi possivel alocar memória para o contexto de configurações");
    }

    ctx->list_configs = g_malloc0(sizeof(SsSettingsClass) * SS_TAG_YAML_LIST_COUNT);
    if (!ctx->list_configs)
    {
        ss_send_notify(SS_ERROR_FATAL, "não foi possivel alocar memória para as configurações");
    }
    
    return ctx;
}


void
ss_hsettings_context_free(SsHSettingsContext **ctx)
{
    if (ctx && *ctx)
    {
        if ((*ctx)->list_configs)
        {
            g_free((*ctx)->list_configs);
            (*ctx)->list_configs = NULL;
        }
        
        g_free(*ctx);
        *ctx = NULL;
    }
}


static GtkWidget *
creat_config_class(GtkWidget *box, const *gchar)
{
    GtkWidget *box_config = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(box), box_config, TRUE, TRUE, 0);

    GtkWidget *label = gtk_label_new(gchar);
    gtk_box_pack_start(GTK_POPOVER(box), label, TRUE, FALSE, 10);
}


void
create_box_settings(SsHSettingsContext **ctx, GtkWidget *button, const SsGeometryWindow *geo_win)
{
    *ctx = ss_hsettings_context_new();
    (*ctx)->button = button;

    (*ctx)->popover_settings = gtk_popover_new(button);
    gtk_popover_set_position(GTK_POPOVER((*ctx)->popover_settings), GTK_POS_BOTTOM);
    gtk_widget_set_size_request((*ctx)->popover_settings, geo_win->width*0.6, geo_win->height);

    (*ctx)->box_settings = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER((*ctx)->popover_settings), (*ctx)->box_settings);

    GtkWidget *scroll_settings = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER((*ctx)->box_settings), scroll_settings);


    //GtkWidget *poweron_label = gtk_label_new("Poweron:");
    //popover_ctx->text_view = gtk_text_view_new();
    //popover_ctx->text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(popover_ctx->text_view));
    //gtk_text_buffer_set_text( popover_ctx->text_buffer, "", -1);
//
    //gtk_text_view_set_editable(GTK_TEXT_VIEW(popover_ctx->text_view), FALSE);
    //gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(popover_ctx->text_view), FALSE);
    //gtk_container_add(GTK_CONTAINER(scroll_settings), popover_ctx->text_view);
    //g_signal_connect(button, "clicked", G_CALLBACK(toggle_viewer_notification), popover_ctx);
}