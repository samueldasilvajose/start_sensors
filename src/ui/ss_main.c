#include "ss_main.h"
#include "ss_types.h"
#include "ss_style.h"
#include "ss_notify.h"
#include "ss_ui_error.h"
#include "ss_dashboard.h"
#include "ss_app_context.h"
#include "ss_history_notify.h"
#include "../controller/ss_controller.h"


const char *name_page[SS_NAME_PAGE_COUNT] = \
{
#define X(name, str) str,
    SS_NAME_PAGE_LIST(X)
#undef X
};


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
generate_main_window_geometry(SsMainWindowContext *win_ctx)
{
    GdkDisplay *display = gdk_display_get_default();
    GdkMonitor *monitor = gdk_display_get_primary_monitor(display);

    if (!monitor)
    {
        ss_send_notify_parse(SS_ERROR_FATAL,
            "Não foi possivel acessar as configurações de display");
        return;
    }

    GdkRectangle geometry = {0};
    gdk_monitor_get_geometry(monitor, &geometry);
    win_ctx->geo.x = geometry.x;
    win_ctx->geo.y = geometry.y;
    win_ctx->geo.width = geometry.width * 0.4;
    win_ctx->geo.height = geometry.height * 0.4;
}


static void
on_menu_clicked(GtkButton *button, gpointer data)
{
    (void) button;
    SsSidebarCtx *sb=data;

    sb->open = !sb->open;
    if(sb->open)
    {
        gtk_widget_show(sb->list);
        gtk_widget_set_size_request(sb->panel, sb->width.mode_visible, -1);
        gtk_image_set_from_icon_name(GTK_IMAGE(sb->button_img), "go-previous-symbolic",
                             GTK_ICON_SIZE_BUTTON);
    }
    else
    {
        gtk_widget_hide(sb->list);
        gtk_widget_set_size_request(sb->panel, sb->width.mode_hidden, -1);
        gtk_image_set_from_icon_name(GTK_IMAGE(sb->button_img), "open-menu-symbolic",
                             GTK_ICON_SIZE_BUTTON);
    }
}


static void
on_row_selected(GtkListBox *box, GtkListBoxRow *row, gpointer data)
{
    (void) box;
    SsSidebarCtx *sb = data;
    
    if(!row)
        return;

    int id_page = gtk_list_box_row_get_index(row);
    if ((unsigned) id_page >= SS_NAME_PAGE_COUNT)
    {
        ss_send_notify_parse(SS_ERROR_ERROR,
            "Não foi possivel acessar as configurações de display");
        return;
    }

    gtk_stack_set_visible_child_name(GTK_STACK(sb->stack), ss_get_name_page(id_page));
}


static void
create_sidebar(SsMainWindowContext *ctx)
{
    if (!ctx)
    {
        ss_send_notify_parse(SS_ERROR_FATAL,
            "contexto da janela principal invalido, SsMainWindowContext * == NULL. line %d",
            __LINE__);
        return;
    }

    SsSidebarCtx *sb = g_new0(SsSidebarCtx, 1);
    ctx->sidebar = sb;

    sb->width.mode_hidden = ctx->geo.width * 0.03;
    sb->width.mode_visible = ctx->geo.width * 0.3;

    sb->panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(sb->panel, sb->width.mode_hidden, -1);

    sb->button_img = gtk_image_new_from_icon_name("open-menu-symbolic", GTK_ICON_SIZE_BUTTON);
    sb->button = gtk_button_new();

    gtk_button_set_image(GTK_BUTTON(sb->button), sb->button_img);
    gtk_box_pack_start(GTK_BOX(sb->panel), sb->button, FALSE, FALSE, 1);

    gtk_widget_set_margin_start(sb->button, 5);

    sb->list = gtk_list_box_new();
    gtk_box_pack_start(GTK_BOX(sb->panel), sb->list, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(ctx->main_frame), sb->panel, FALSE, FALSE, 0);

    sb->overlay = gtk_overlay_new();
    
    gtk_widget_set_hexpand(sb->overlay, TRUE);
    gtk_widget_set_vexpand(sb->overlay, TRUE);

    gtk_box_pack_start(GTK_BOX(ctx->main_frame), sb->overlay, TRUE, TRUE, 0);

    sb->stack = gtk_stack_new();

    gtk_widget_set_hexpand(sb->stack, TRUE);
    gtk_widget_set_vexpand(sb->stack, TRUE);

    gtk_stack_set_transition_type(GTK_STACK(sb->stack),
        GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);

    gtk_container_add(GTK_CONTAINER(sb->overlay), sb->stack);

    g_signal_connect(sb->button, "clicked", G_CALLBACK(on_menu_clicked), sb);
    g_signal_connect(sb->list, "row-selected", G_CALLBACK(on_row_selected), sb);
}


static gboolean
continuous_status_checking(gpointer data)
{
    SsDashboardContext *dashboard = data;

    ss_configs_t configs = {0};
    ss_controller_get_configs(&configs);

    if (configs.power[SS_READ_STATE].to_use)
    {
        GAsyncQueue *queue = ss_read_can_thread_start();
        ss_send_command_thread(queue, SS_COMMAND_THREAD_REPEAT);
    }

    g_timeout_add(1500, ss_desired_power_status, (gpointer) dashboard);
    
    return TRUE;
}


//estrutura da janela principal
static void
init_main_window (GtkApplication *app, gpointer  data)
{
    //preenchendo dados do contexto do app
    SsAppContext *app_ctx = (SsAppContext *) data;
    app_ctx->window = ss_main_window_context_new();
    app_ctx->window->controller = ss_init_controller();

    SsMainWindowContext *ctx = app_ctx->window;
    generate_main_window_geometry(ctx);

    //configurações da janela
    ctx->main_win = gtk_application_window_new(app); //cria janela principal
    gtk_window_set_title (GTK_WINDOW (ctx->main_win), SS_NAME_MAIN_WINDOW); //define nome da janela
    gtk_window_set_resizable(GTK_WINDOW(ctx->main_win), FALSE); //desativa mudança no tamanho da gui
    gtk_window_set_default_size (GTK_WINDOW (ctx->main_win), ctx->geo.width, ctx->geo.height);

    //frame principal
    ctx->main_frame = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(ctx->main_win), ctx->main_frame);

    create_sidebar(ctx);

    ss_init_style();
    ss_start_notify(&app_ctx->notify, ctx);
    
    ss_create_dashboard(&app_ctx->dashboard, ctx);
    ss_create_history_notify(&app_ctx->history_notify, ctx);

    gtk_widget_show_all(ctx->main_win);

    set_default_state(app_ctx->dashboard);

    gtk_widget_hide(ctx->sidebar->list);
    gtk_stack_set_visible_child_name(GTK_STACK(ctx->sidebar->stack),
        ss_get_name_page(SS_NAME_PAGE_DASHBOARD));

    g_timeout_add(2000, continuous_status_checking, (gpointer) app_ctx->dashboard);
}


static void
close_app(SsController *ctl, gpointer data, gpointer user_data)
{
    (void) ctl;
    (void) data;

    //encerrando corretamente o app
    GtkApplication *app = (GtkApplication *) user_data;
    g_application_quit(G_APPLICATION(app));
}


int
ss_rum_app(int argc, char **argv)
{
    //contexto do app
    SsAppContext *app_ctx = ss_app_context_new();
    app_ctx->app = gtk_application_new("ai.lumerobotics.StartSensors", G_APPLICATION_FLAGS_NONE);

    //definindo handle para start do app
    g_signal_connect (app_ctx->app, "activate", G_CALLBACK(init_main_window), app_ctx);

    //definindo handle para tratamento do encerramento do app
    g_signal_connect (ss_init_controller(), ss_singnal_to_string(SS_SIGNAL_UPDATE_BACKEND_CLOSE_APPLICATION),
                        G_CALLBACK(close_app), (gpointer) app_ctx->app);

    if (g_application_run(G_APPLICATION(app_ctx->app), argc, argv) != 0)
    {
        //implementação errno para ui, definindo erro global
        ss_set_error(SS_ERROR_FATAL);
    }

    //liberando memoria do app
    g_object_unref(app_ctx->app);
    ss_app_context_free(&app_ctx);

    return ss_get_error();
}
