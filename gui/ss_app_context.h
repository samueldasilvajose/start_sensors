#ifndef SS_APP_CONTEXT_H
#define SS_APP_CONTEXT_H

typedef struct _GtkApplication GtkApplication;
typedef struct _SsMainWindowContext SsMainWindowContext;

typedef struct _SsHeaderContext SsHeaderContext;
typedef struct _SsBodyContext SsBodyContext;

typedef struct
{
    GtkApplication *app;
    SsMainWindowContext *window;

    SsHeaderContext *header;
    SsBodyContext   *body;
} SsAppContext;

void ss_app_context_free(SsAppContext **ctx);
SsAppContext *ss_app_context_new();

#endif //SS_APP_CONTEXT_H