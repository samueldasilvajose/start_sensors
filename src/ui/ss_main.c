#include <gtk/gtk.h>

#include "ss_main.h"
#include "ss_body.h"
#include "ss_types.h"
#include "ss_style.h"
#include "ss_header.h"
#include "ss_ui_error.h"
#include "ss_app_context.h"
#include "../controller/ss_controller.h"


SsMainWindowContext *
ss_main_window_context_new()
{
    return g_malloc0(sizeof(SsMainWindowContext));
}


void
ss_main_window_context_free(SsMainWindowContext **ctx)
{
    if (ctx && *ctx)
    {
        g_free(*ctx);
        *ctx = NULL;
    }
}


static void
generates_main_window_geometry(SsMainWindowContext *win_ctx)
{
    GdkDisplay *display = gdk_display_get_default();
    GdkMonitor *monitor = gdk_display_get_primary_monitor(display);

    if (!monitor)
    {
        ss_send_notify(SS_ERROR_FATAL, "Não foi possivel acessar as configurações de display");
    }

    GdkRectangle geometry = {0};
    gdk_monitor_get_geometry(monitor, &geometry);
    win_ctx->geo.x = geometry.x;
    win_ctx->geo.y = geometry.y;
    win_ctx->geo.width = geometry.width * 0.4;
    win_ctx->geo.height = geometry.height * 0.4;
}


//criar contexto passar por callback
static void
init_main_window (GtkApplication *app, gpointer  data)
{
    SsAppContext *app_ctx = (SsAppContext *) data;
    app_ctx->window = ss_main_window_context_new();
    app_ctx->window->controller = ss_init_controller();

    SsMainWindowContext *ctx = app_ctx->window;
    generates_main_window_geometry(ctx);

    //configurações da janela
    ctx->main_win = gtk_application_window_new (app); //cria janela principal
    gtk_window_set_title (GTK_WINDOW (ctx->main_win), SS_NAME_MAIN_WINDOW); //define nome da janela
    gtk_window_set_resizable(GTK_WINDOW(ctx->main_win), FALSE); //desativa mudança no tamanho da gui
    gtk_window_set_default_size (GTK_WINDOW (ctx->main_win), ctx->geo.width, ctx->geo.height);

    //frame principal
    ctx->main_frame = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(ctx->main_win), ctx->main_frame);
    gtk_widget_set_hexpand(ctx->main_frame, TRUE);
    gtk_widget_set_vexpand(ctx->main_frame, TRUE);

    ss_init_style();
    ss_create_header(&app_ctx->header, ctx);
    ss_create_body(&app_ctx->body, ctx);
    ss_create_footer(app_ctx->body, ctx);

    gtk_widget_show_all(ctx->main_win);

    set_default_state(app_ctx->body);
    
    ss_read_can_state();
    g_timeout_add(1500, ss_desired_power_status, (gpointer) app_ctx->body);
}


static void
close_app(SsController *ctl, gpointer data, gpointer user_data)
{
    (void) ctl;
    (void) data;

    GtkApplication *app = (GtkApplication *) user_data;
    g_application_quit(G_APPLICATION(app));
}


int
ss_rum_app(int argc, char **argv)
{
    SsAppContext *app_ctx = ss_app_context_new();
    app_ctx->app = gtk_application_new("ai.lumerobotics.StartSensors", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app_ctx->app, "activate", G_CALLBACK(init_main_window), app_ctx);
    g_signal_connect (ss_init_controller(), ss_singnal_to_string(SS_SIGNAL_UPDATE_BACKEND_CLOSE_APPLICATION),
                        G_CALLBACK(close_app), (gpointer) app_ctx->app);

    if (g_application_run(G_APPLICATION(app_ctx->app), argc, argv) != 0)
    {
        ss_set_error(SS_ERROR_FATAL);
    }

    g_object_unref(app_ctx->app);
    ss_app_context_free(&app_ctx);

    return ss_get_error();
}
