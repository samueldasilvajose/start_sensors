#include "ss_notify.h"
#include "ss_ui_error.h"
#include "../controller/ss_controller.h"


SsNotificationContext *
ss_notify_context_new()
{
    return g_malloc0(sizeof(SsNotificationContext));
}


void
ss_notify_context_free(SsNotificationContext **ctx)
{
    if (ctx && *ctx)
    {
        g_free(*ctx);
        *ctx = NULL;
    }
}


static gboolean
hide_notification(gpointer data)
{
    SsNotificationContext *notify_ctx = (SsNotificationContext *) data;

    notify_ctx->timeout_id = 0;
    gtk_widget_hide(GTK_WIDGET(notify_ctx->label.widget));

    return FALSE;
}


void
ss_show_notify(SsController *ctl, gpointer data, gpointer user_data)
{
    (void) ctl;

    SsNotifyMsg *notify = (SsNotifyMsg *) data;
    SsNotificationContext *ctx = (SsNotificationContext *) user_data;

    if (notify->err <= SS_ERROR_INFO)
        return;
    
    gtk_label_set_text(GTK_LABEL(ctx->label.widget), notify->msg);
    ss_update_style_widget(&ctx->label);

    gtk_widget_show(ctx->label.widget);

    if (ctx->timeout_id > 0)
        g_source_remove(ctx->timeout_id);
    
    ctx->timeout_id = g_timeout_add(3000, hide_notification, ctx);
}


void
ss_start_notify(SsNotificationContext **notify_ctx, SsMainWindowContext *win_ctx)
{
    *notify_ctx = ss_notify_context_new();

    //preenchendo elementos da notificação
    (*notify_ctx)->label.style = SS_STYLE_BLACK;
    (*notify_ctx)->label._first_style = SS_STYLE_NONE;
    (*notify_ctx)->label.widget = gtk_label_new("");

    gtk_overlay_add_overlay(GTK_OVERLAY(win_ctx->sidebar->overlay), (*notify_ctx)->label.widget);
    gtk_widget_set_halign((*notify_ctx)->label.widget, GTK_ALIGN_CENTER);
    gtk_widget_set_valign((*notify_ctx)->label.widget, GTK_ALIGN_START);

    gtk_widget_set_margin_top((*notify_ctx)->label.widget, 0);

    gtk_widget_hide((*notify_ctx)->label.widget);

    g_signal_connect (win_ctx->controller, ss_singnal_to_string(SS_SIGNAL_UPDATE_BACKEND_NOTIFICATION),
                    G_CALLBACK(ss_show_notify), *notify_ctx);
}
