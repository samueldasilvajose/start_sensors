#ifndef SS_MAIN_H
#define SS_MAIN_H

#include "ss_types.h"

#define SS_NAME_MAIN_WINDOW "Start Sensors"

typedef struct _SsController SsController;
typedef struct _SsMainWindowContext
{
    GtkWidget *main_win;
    GtkWidget *main_frame;
    SsGeometryWindow geo;
    SsController *controller;
} SsMainWindowContext;


void ss_main_window_context_free(SsMainWindowContext **ctx);
int ss_rum_app (int argc, char **argv);
SsMainWindowContext *ss_main_window_context_new();

#endif //SS_MAIN_H