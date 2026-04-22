#include <gtk/gtk.h>

#include "ss_body.h"
#include "ss_header.h"
#include "ss_ui_error.h"
#include "../controller/ss_controller.h"


SsBodyContext *
ss_body_context_new()
{
    return g_malloc0(sizeof(SsBodyContext));
}


void
ss_body_context_free(SsBodyContext **ctx)
{
    if (ctx && *ctx)
    {
        g_free(*ctx);
        *ctx = NULL;
    }
}


int
ss_set_power_theme(SsBodyContext *ctx, int theme, const char *msg)
{
    SsTheme *t = &ctx->body[theme];
    if ((SsThemeType) theme == ctx->current_theme)
    {
        return 1;
    }

    t->label._first_style = ctx->body[ctx->current_theme].label.style;
    t->frame._first_style = ctx->body[ctx->current_theme].frame._first_style;
    
    ss_apply_theme(t);
    gtk_label_set_text(GTK_LABEL(t->label.widget), msg);
    ctx->current_theme = theme;

    return 0;
}


static void
poweron_sensors(GtkWidget *button, gpointer data)
{
    (void) button;

    SsBodyContext *ctx = data;
    if (SS_THEME_POWERON == ctx->current_theme)
    {
        return;
    }

    ss_poweron_sensors();
    
    ss_check_ips();
    ss_set_power_theme(data, SS_THEME_POWERON, "Sensores ligados!");
}


static void
poweroff_sensors(GtkWidget *button, gpointer data)
{
    (void) button;
    
    SsBodyContext *ctx = data;
    if (SS_THEME_POWEROFF == ctx->current_theme)
    {
        return;
    }

    ss_set_power_theme(data, SS_THEME_POWEROFF, "Sensores desligados!");
    ss_poweroff_sensors();
    ss_check_ips_encerrer();
}


void
set_default_state(SsBodyContext *ctx)
{
    ss_send_stack_msg(SS_STACK_COMPONENTS_EMPTY);
    ss_set_power_theme(ctx, SS_THEME_POWEROFF, "");
}


static void
desired_power_status(SsBodyContext *ctx)
{
    ss_configs_t data = {0};
    ss_controller_get_configs(&data);

    if (!data.power[SS_READ_STATE].to_use)
    {
        set_default_state(ctx);
        return;
    }

    int current_state = -1;
    if ((current_state = ss_read_state_sensors()) >= 0)
    {
        if (current_state)
        {
            poweron_sensors(NULL, ctx);
        }
        else
        {
            poweroff_sensors(NULL, ctx);
        }
    }
    else
    {
        char buf[512];
        snprintf(buf, sizeof(buf), "Não foi encontrado a mensagem de status (msg: %x, frame: 0x0%x)",
                    data.power[SS_READ_STATE].msg.can_id, data.power[SS_READ_STATE].msg.data[0]);
        set_default_state(ctx);
        ss_send_notify(SS_ERROR_ERROR, buf);
    }
}


gboolean
ss_desired_power_status(gpointer data)
{
    SsBodyContext *ctx = (SsBodyContext *) data;
    desired_power_status(ctx);
    return FALSE;
}


void
ss_create_body(SsBodyContext **body_ctx, SsMainWindowContext *win_ctx)
{
    *body_ctx = ss_body_context_new();

    SsTheme *on = &(*body_ctx)->body[SS_THEME_POWERON];
    on->frame.style = SS_STYLE_BG_WHITE;
    on->frame._first_style = SS_STYLE_NONE;
    on->frame.widget = win_ctx->main_frame;
    on->wallpaper.wallpaper_path = IMG_SENSORS_ON;
    on->label.style = SS_STYLE_GREEN;

    SsTheme *off = &(*body_ctx)->body[SS_THEME_POWEROFF];
    (*body_ctx)->current_theme = SS_THEME_COUNT;
    off->frame.style = SS_STYLE_BG_GRAY;
    off->frame._first_style = SS_STYLE_NONE;
    off->frame.widget = win_ctx->main_frame;
    off->wallpaper.wallpaper_path = IMG_SENSORS_OFF;
    off->label.style = SS_STYLE_RED;

    GtkWidget *img = gtk_image_new();
    on->wallpaper.img = off->wallpaper.img = img;
    on->wallpaper.wallpaper_geometry = off->wallpaper.wallpaper_geometry = win_ctx->geo;
    gtk_widget_set_size_request(img, win_ctx->geo.width*0.1, win_ctx->geo.height);

    GtkWidget *lebal_status = gtk_label_new("");
    gtk_widget_set_halign(lebal_status, GTK_ALIGN_CENTER);

    gtk_widget_set_margin_bottom(lebal_status, 30);
    on->label.widget = off->label.widget = lebal_status;
    
    gtk_box_pack_start(GTK_BOX(win_ctx->main_frame), img, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(win_ctx->main_frame), lebal_status, FALSE, FALSE, 0);
}


void
ss_create_footer(SsBodyContext *body_ctx, SsMainWindowContext *win_ctx)
{
    GtkWidget *footer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(win_ctx->main_frame), footer, FALSE, FALSE, 20);
    gtk_widget_set_halign(footer, GTK_ALIGN_CENTER);

    // Botão ligar
    GtkWidget *but_power_sensors = gtk_button_new_with_label("power_on");
    g_signal_connect(but_power_sensors, "clicked", G_CALLBACK(poweron_sensors), (gpointer) body_ctx);

    // Botão desligar
    GtkWidget *but_poweroff_sensors = gtk_button_new_with_label("power_off");
    g_signal_connect(but_poweroff_sensors, "clicked", G_CALLBACK(poweroff_sensors), (gpointer) body_ctx);

    // Adiciona os botões no footer
    gtk_box_pack_start(GTK_BOX(footer), but_power_sensors, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(footer), but_poweroff_sensors, FALSE, FALSE, 10);

    // Centraliza verticalmente os botões dentro do box_footer
    gtk_widget_set_valign(but_power_sensors, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(but_poweroff_sensors, GTK_ALIGN_CENTER);
}
