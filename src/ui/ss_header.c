#include <gtk/gtk.h>

#include "ss_main.h"
#include "ss_header.h"
#include "ss_ui_error.h"
#include "../controller/ss_controller.h"

//adicionando configs
//#include "ss_header_settings.h"


SsHeaderContext *
ss_header_context_new()
{
    return g_malloc0(sizeof(SsHeaderContext));
}


void
ss_header_context_free(SsHeaderContext **ctx)
{
    if (ctx && *ctx)
    {
        g_free(*ctx);
        *ctx = NULL;
    }
}


void
ss_define_item_in_stack(SsController *ctl, gpointer data, gpointer user_data)
{
    (void) ctl;

    SsStackComponentsType type = *((SsStackComponentsType *) data);
    SsStackContext *ctx = (SsStackContext *) user_data;
    gtk_stack_set_visible_child_name(GTK_STACK(ctx->stack), ctx->components[type].label);
}


static gboolean
hide_notification(gpointer data)
{
    SsNotificationContext *notify_ctx = (SsNotificationContext *) data;
    notify_ctx->timeout_id = 0;
    gtk_widget_hide(GTK_WIDGET(notify_ctx->noti_label.widget));
    return FALSE;
}


void
ss_show_notify(SsController *ctl, gpointer data, gpointer user_data)
{
    (void) ctl;

    SsNotifyMsg *noti = (SsNotifyMsg *) data;
    SsNotificationContext *ctx = (SsNotificationContext *) user_data;

    if (noti->err <= SS_ERROR_INFO)
    {
        return;
    }
    
    gtk_label_set_text(GTK_LABEL(ctx->noti_label.widget), noti->msg);
    ss_update_style_widget(&ctx->noti_label);
    gtk_widget_show(ctx->noti_label.widget);

    if (ctx->timeout_id > 0)
    {
        g_source_remove(ctx->timeout_id);
    }
    
    ctx->timeout_id = g_timeout_add(3000, hide_notification, ctx);
}


void
ss_write_notification_viewer(SsController *ctl, gpointer data, gpointer user_data)
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


static void
create_notification_widget(SsNotificationContext *notify_ctx)
{
    notify_ctx->noti_label.style = SS_STYLE_BLACK;
    notify_ctx->noti_label._first_style = SS_STYLE_NONE;
    notify_ctx->noti_label.widget = gtk_label_new("");
    gtk_overlay_add_overlay(GTK_OVERLAY(notify_ctx->noti_over), notify_ctx->noti_label.widget);
    gtk_widget_set_halign(notify_ctx->noti_label.widget, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(notify_ctx->noti_label.widget, GTK_ALIGN_START);

    gtk_widget_set_margin_top(notify_ctx->noti_label.widget, 0);

    gtk_widget_hide(notify_ctx->noti_label.widget);
}


static void
toggle_viewer_notification(GtkButton *button, gpointer data)
{
    (void) button;

    SsHistoryNotifyContext *popover = (SsHistoryNotifyContext *) data;
    if (gtk_widget_get_visible(popover->popover))
    {
        gtk_widget_hide(popover->popover);
    }
    else
    {
        GtkTextMark *mark = gtk_text_buffer_get_insert(popover->text_buffer);
        gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(popover->text_view), mark);
        gtk_widget_show_all(popover->popover);
    }
}


static void
create_viewer_notification(GtkWidget *button, SsHistoryNotifyContext *popover_ctx, const SsGeometryWindow *geo_win)
{
    popover_ctx->popover = gtk_popover_new(button);
    gtk_popover_set_position(GTK_POPOVER(popover_ctx->popover), GTK_POS_BOTTOM);
    gtk_widget_set_size_request(popover_ctx->popover, geo_win->width*0.5, geo_win->height);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(popover_ctx->popover), scroll);

    popover_ctx->text_view = gtk_text_view_new();
    popover_ctx->text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(popover_ctx->text_view));
    gtk_text_buffer_set_text( popover_ctx->text_buffer, "", -1);

    gtk_text_view_set_editable(GTK_TEXT_VIEW(popover_ctx->text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(popover_ctx->text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scroll), popover_ctx->text_view);
    g_signal_connect(button, "clicked", G_CALLBACK(toggle_viewer_notification), popover_ctx);
}


static void
set_items_in_stack(SsStackContext *data)
{
    static const char *name_img[SS_STACK_COMPONENTS_COUNT] = \
    {
        [SS_STACK_COMPONENTS_OK] = "dialog-ok",
        [SS_STACK_COMPONENTS_ERROR] = "process-stop", 
        [SS_STACK_COMPONENTS_EMPTY] = "dialog-question", 
        [SS_STACK_COMPONENTS_WARNING] = "dialog-warning", 
        [SS_STACK_COMPONENTS_LOADING] = NULL, 
    };

    for (int i = 0; i < SS_STACK_COMPONENTS_COUNT; i++)
    {
        const char *aux = name_img[i];
        if (aux == NULL)
        {
           continue;
        }
        
        SsStackComponent *component = &data->components[i];
        component->img = gtk_image_new_from_icon_name(aux, GTK_ICON_SIZE_LARGE_TOOLBAR);
        component->label = ss_stack_components_to_string(i);
        gtk_stack_add_named(GTK_STACK(data->stack), component->img, component->label);
    }

    SsStackComponent *component = &data->components[SS_STACK_COMPONENTS_LOADING];
    component->img = gtk_spinner_new();
    component->label = ss_stack_components_to_string(SS_STACK_COMPONENTS_LOADING);
    gtk_spinner_start(GTK_SPINNER(component->img));
    gtk_widget_set_size_request(component->img, 30, 30);
    gtk_stack_add_named(GTK_STACK(data->stack), component->img, component->label);
}


void
ss_create_header(SsHeaderContext **header_ctx, SsMainWindowContext *win_ctx)
{
    *header_ctx = ss_header_context_new();

    (*header_ctx)->notification.noti_over = gtk_overlay_new();
    gtk_box_pack_start(GTK_BOX(win_ctx->main_frame), (*header_ctx)->notification.noti_over, FALSE, FALSE, 10);

    (*header_ctx)->header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER((*header_ctx)->notification.noti_over), (*header_ctx)->header);

    SsStackContext *stack = &(*header_ctx)->stack;

    stack->stack = gtk_stack_new();
    gtk_box_pack_start(GTK_BOX((*header_ctx)->header), stack->stack, TRUE, TRUE, 20);
    gtk_widget_set_halign(stack->stack, GTK_ALIGN_START);  // direita
    gtk_widget_set_tooltip_text(stack->stack, "Status dos sensores");

    set_items_in_stack(stack);
    g_signal_connect (win_ctx->controller, ss_singnal_to_string(SS_SIGNAL_UPDATE_BACKEND_STACK),
        G_CALLBACK(ss_define_item_in_stack), &(*header_ctx)->stack);

    //adicionando configs
    //GtkWidget *button_config = gtk_button_new_from_icon_name("preferences-system", GTK_ICON_SIZE_BUTTON);
    //gtk_widget_set_tooltip_text(button_config, "Configurações");
    //gtk_box_pack_start(GTK_BOX((*header_ctx)->header), button_config, FALSE, FALSE, 0);

    GtkWidget *button_info = gtk_button_new_from_icon_name("dialog-information", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(button_info, "Historico de notificações");
    gtk_box_pack_start(GTK_BOX((*header_ctx)->header), button_info, FALSE, FALSE, 0);

    SsHistoryNotifyContext *popover_ctx = &(*header_ctx)->history_notify;
    create_viewer_notification(button_info, popover_ctx, &win_ctx->geo);
    g_signal_connect (win_ctx->controller, ss_singnal_to_string(SS_SIGNAL_UPDATE_BACKEND_NOTIFICATION),
                G_CALLBACK(ss_write_notification_viewer),  popover_ctx);

    //definir borda
    GtkWidget *empty_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(empty_area, 10, 10);  // largura x altura
    gtk_box_pack_start(GTK_BOX((*header_ctx)->header), empty_area, FALSE, FALSE, 0);

    create_notification_widget(&(*header_ctx)->notification);
    g_signal_connect (win_ctx->controller, ss_singnal_to_string(SS_SIGNAL_UPDATE_BACKEND_NOTIFICATION),
                    G_CALLBACK(ss_show_notify), &(*header_ctx)->notification);
}
