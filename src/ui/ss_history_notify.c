#include "ss_ui_error.h"
#include "ss_history_notify.h"
#include "../controller/ss_controller.h"

SsHistoryNotifyContext *
ss_history_notify_context_new()
{
    return g_malloc0(sizeof(SsHistoryNotifyContext));
}


void
ss_history_notify_context_free(SsHistoryNotifyContext **ctx)
{
    if (ctx && *ctx)
    {
        g_free(*ctx);
        *ctx = NULL;
    }
}


static void
write_history_notify(SsController *ctl, gpointer data, gpointer user_data)
{
    (void) ctl;

    SsNotifyMsg *noti = (SsNotifyMsg *) data;
    SsHistoryNotifyContext *ctx = (SsHistoryNotifyContext *) user_data;

    if (!ctx->text_buffer)
    {
        ctx->text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ctx->text_view));
    }

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%s: %s\n", ss_error_to_string(noti->err), noti->msg);

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(ctx->text_buffer, &end);
    gtk_text_buffer_insert(ctx->text_buffer, &end, buffer, -1);
}


void
ss_create_history_notify(SsHistoryNotifyContext **history_notify_ctx, SsMainWindowContext *win_ctx)
{
    SsHistoryNotifyContext *hn_ctx = ss_history_notify_context_new();
    *history_notify_ctx = hn_ctx;

    const char *np = ss_get_name_page(SS_NAME_PAGE_HISTORY);

    GtkWidget *label_np = gtk_label_new(np);
    gtk_list_box_insert(GTK_LIST_BOX(win_ctx->sidebar->list), label_np, SS_NAME_PAGE_HISTORY);

    hn_ctx->page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_stack_add_named(GTK_STACK(win_ctx->sidebar->stack),
                            hn_ctx->page, np);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(hn_ctx->page), scroll);

    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_widget_set_vexpand(scroll, TRUE);   

    hn_ctx->text_view = gtk_text_view_new();
    hn_ctx->text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(hn_ctx->text_view));
    gtk_text_buffer_set_text( hn_ctx->text_buffer, "", -1);

    gtk_text_view_set_editable(GTK_TEXT_VIEW(hn_ctx->text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(hn_ctx->text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scroll), hn_ctx->text_view);

    g_signal_connect (win_ctx->controller, ss_singnal_to_string(SS_SIGNAL_UPDATE_BACKEND_NOTIFICATION),
                G_CALLBACK(write_history_notify),  hn_ctx);
}
