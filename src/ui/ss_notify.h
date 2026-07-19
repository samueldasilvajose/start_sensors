#ifndef SS_NOTIFY_H
#define SS_NOTIFY_H

#include "ss_main.h"
#include "ss_style.h"


typedef struct _SsNotificationContext
{
    unsigned int timeout_id;
    SsWidgetStyleContext label;
} SsNotificationContext;


void ss_start_notify(SsNotificationContext **notify_ctx, SsMainWindowContext *win_ctx);
void ss_show_notify(SsController *ctl, gpointer data, gpointer user_data);

SsNotificationContext *ss_notify_context_new();
void ss_notify_context_free(SsNotificationContext **ctx);

#endif //SS_NOTIFY_H