#ifndef SS_HISTORY_NOTIFY_H
#define SS_HISTORY_NOTIFY_H

#include "ss_main.h"
#include "ss_types.h"


typedef struct _SsHistoryNotifyContext
{
    GtkWidget *page;
    GtkWidget *text_view;
    GtkTextBuffer *text_buffer;
} SsHistoryNotifyContext;

void ss_create_history_notify(SsHistoryNotifyContext **header_ctx, SsMainWindowContext *win_ctx);

SsHistoryNotifyContext *ss_history_notify_context_new();
void ss_history_notify_context_free(SsHistoryNotifyContext **ctx);


#endif //SS_HISTORY_NOTIFY_H