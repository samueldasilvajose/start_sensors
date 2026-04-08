#include <gtk/gtk.h>

#include "ss_header.h"
#include "ss_ui_error.h"
#include "ss_header_settings.h"

#include "../core/ss_core_types.h"
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


static void
toggle_viewer_settings(GtkButton *button, gpointer data)
{
    (void) button;

    SsHSettingsContext *popover = (SsHSettingsContext *) data;
    if (gtk_widget_get_visible(popover->popover_settings))
    {
        gtk_widget_hide(popover->popover_settings);
    }
    else
    {
        gtk_widget_show_all(popover->popover_settings);
    }
}


static void
on_switch_changed(GObject *switch_widget, GParamSpec *pspec, gpointer data)
{
    (void) pspec;

    struct FlagAndConfig *ptr = (struct FlagAndConfig *) data;
    bool *state = (ptr->flag == FlagSettingIp ? &ptr->sh_ip.to_use : &ptr->sh_power.to_use);
    *state = gtk_switch_get_active(GTK_SWITCH(switch_widget));
}


static GtkWidget *
create_new_frame_settings(bool to_use, const char *box_name, struct FlagAndConfig *data)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

    GtkWidget *label = gtk_label_new(box_name);
    gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 10);

    GtkWidget *sw = gtk_switch_new();
    gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 10);
    gtk_switch_set_active(GTK_SWITCH(sw), to_use);
    g_signal_connect(sw, "notify::active", G_CALLBACK(on_switch_changed), (gpointer) data);

    GtkWidget *config = gtk_button_new_from_icon_name("preferences-system", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(config, "Alterar");
    gtk_box_pack_start(GTK_BOX(box), config, FALSE, FALSE, 10);
}


static void
create_cans_settings(GtkWidget *box_settings, short qtd_cans)
{
    static struct FlagAndConfig data[qtd_cans];
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(box_settings), header_box, FALSE, FALSE, 10);

    GtkWidget *label = gtk_label_new("Power");
    gtk_box_pack_start(GTK_BOX(header_box), label, TRUE, TRUE, 10);

    GtkWidget *config = gtk_button_new_with_label("∨");
    gtk_box_pack_start(GTK_BOX(header_box), config, TRUE, TRUE, 10);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(box_settings), box, FALSE, FALSE, 10);

    const ss_configs_t configs;
    ss_controller_get_configs(&configs);

    for (short i = 0; i < qtd_cans; i++)
    {
        GtkWidget *tmp = create_new_frame_settings()
    }
}


void
ss_create_box_settings(SsHSettingsContext *ctx, GtkWidget *button, const SsGeometryWindow *geo_win)
{
    ctx = ss_hsettings_context_new();
    ctx->button = button;

    //base
    ctx->popover_settings = gtk_popover_new(button);
    gtk_popover_set_position(GTK_POPOVER(ctx->popover_settings), GTK_POS_BOTTOM);
    gtk_widget_set_size_request(ctx->popover_settings, geo_win->width*0.6, geo_win->height*0.99);
    g_signal_connect(button, "clicked", G_CALLBACK(toggle_viewer_settings), ctx);

    GtkWidget *scroll_settings = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(ctx->popover_settings), scroll_settings);

    ctx->overlay_settings = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(scroll_settings), ctx->overlay_settings);

    ctx->box_settings = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(ctx->overlay_settings), ctx->box_settings);

    //header
    GtkWidget *header_settings = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(ctx->box_settings), header_settings, FALSE, FALSE, 10);

    GtkWidget *title = gtk_label_new("Settings");
    gtk_box_pack_start(GTK_BOX(header_settings), title, TRUE, TRUE, 10);

    GtkWidget *new_sensor = gtk_button_new_with_label("New sensor");
    gtk_box_pack_start(GTK_BOX(header_settings), new_sensor, FALSE, TRUE, 10);
    gtk_widget_set_valign(new_sensor, GTK_ALIGN_END);

    //body
    GtkWidget *body_settings = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(ctx->box_settings), body_settings, TRUE, TRUE, 10);
    gtk_widget_set_valign(new_sensor, GTK_ALIGN_CENTER);

    create_cans_settings(ctx->box_settings, SS_POWER_COUNT);

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