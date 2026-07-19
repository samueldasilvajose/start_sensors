#ifndef SS_MAIN_H
#define SS_MAIN_H

#include "ss_types.h"

#define SS_NAME_MAIN_WINDOW "Start Sensors"

#define SS_NAME_PAGE_LIST(x) \
    x(DASHBOARD, "Dashboard") \
    x(HISTORY, "History") \
    x(SETTINGS, "Settings")


typedef enum
{
#define X(name, str) SS_NAME_PAGE_##name,
    SS_NAME_PAGE_LIST(X)
#undef X
    SS_NAME_PAGE_COUNT
} SsNamePageId;


typedef struct
{
    struct
    {
        int mode_hidden;
        int mode_visible;
    } width;

    gboolean open;

    GtkWidget *list;
    GtkWidget *stack;
    GtkWidget *panel;
    GtkWidget *button;
    GtkWidget *overlay;
    GtkWidget *button_img;
} SsSidebarCtx;

typedef struct _SsController SsController;
typedef struct _SsMainWindowContext
{
    GtkWidget *main_win;
    GtkWidget *main_frame;
    SsSidebarCtx *sidebar;
    SsController *controller;
    SsGeometryWindow geo;
} SsMainWindowContext;


void ss_main_window_context_free(SsMainWindowContext **ctx);
int ss_rum_app (int argc, char **argv);
SsMainWindowContext *ss_main_window_context_new();


extern const char *name_page[SS_NAME_PAGE_COUNT];

static inline const char *
ss_get_name_page(SsNamePageId id)
{
    if ((unsigned) id >= SS_NAME_PAGE_COUNT)
        return NULL;

    return name_page[id];
}

#endif //SS_MAIN_H