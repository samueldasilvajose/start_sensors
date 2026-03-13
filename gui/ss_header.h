#ifndef SS_HEADER_H
#define SS_HEADER_H

#include "ss_main.h"
#include "ss_style.h"
#include "ss_types.h"

#define SS_STACK_COMPONENTS_LIST(X) \
    X(OK,       "ok")              \
    X(ERROR,    "error")           \
    X(EMPTY,    "empty")           \
    X(WARNING,  "warning")         \
    X(LOADING,  "loading")


typedef enum
{
#define X(name, str) SS_STACK_COMPONENTS_##name,
    SS_STACK_COMPONENTS_LIST(X)
#undef X
    SS_STACK_COMPONENTS_COUNT
} SsStackComponentsType;

typedef struct
{
    GtkWidget *img;
    const char *label;
} SsStackComponent;

typedef struct
{
    GtkWidget *stack;
    SsStackComponent components[SS_STACK_COMPONENTS_COUNT];
} SsStackContext;

typedef struct
{
    unsigned int timeout_id;
    GtkWidget *noti_over;
    SsWidgetStyleContext noti_label;
} SsNotificationContext;

typedef struct
{
    GtkWidget *popover;
    GtkWidget *text_view;
    GtkTextBuffer *text_buffer;
} SsHistoryNotifyContext;

typedef struct _SsHeaderContext
{
    GtkWidget *header;
    SsStackContext stack;
    SsNotificationContext notification;
    SsHistoryNotifyContext history_notify;
} SsHeaderContext;

void ss_create_header(SsHeaderContext **header_ctx, SsMainWindowContext *win_ctx);
void ss_show_notify(SsController *ctl, gpointer data, gpointer user_data);
void ss_define_item_in_stack(SsController *ctl, gpointer data, gpointer user_data);
void ss_header_context_free(SsHeaderContext **ctx);
void ss_write_notification_viewer(SsController *ctl, gpointer data, gpointer user_data);

SsHeaderContext *ss_header_context_new();


static inline const char *
ss_stack_components_to_string(SsStackComponentsType type)
{
    static const char *_ss_stack_components_strings[] =
{
#define X(name, str) str,
    SS_STACK_COMPONENTS_LIST(X)
#undef X
};

    if ((unsigned) type >= SS_STACK_COMPONENTS_COUNT)
    {
        return "";
    }
    return _ss_stack_components_strings[type];
}

#endif //SS_HEADER_H