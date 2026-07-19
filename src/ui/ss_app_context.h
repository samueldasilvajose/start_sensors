#ifndef SS_APP_CONTEXT_H
#define SS_APP_CONTEXT_H

typedef struct _GtkApplication GtkApplication;
typedef struct _SsMainWindowContext SsMainWindowContext;

typedef struct _SsNotificationContext SsNotificationContext;
typedef struct _SsHistoryNotifyContext SsHistoryNotifyContext;
typedef struct _SsHeaderContext SsHeaderContext;
typedef struct _SsDashboardContext SsDashboardContext;

typedef struct
{
    GtkApplication *app;
    SsMainWindowContext *window;

    SsDashboardContext      *dashboard;
    SsNotificationContext   *notify;
    SsHistoryNotifyContext  *history_notify;
} SsAppContext;

void ss_app_context_free(SsAppContext **ctx);
SsAppContext *ss_app_context_new();

#endif //SS_APP_CONTEXT_H