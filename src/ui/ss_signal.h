#ifndef SS_SIGNAL_H
#define SS_SIGNAL_H

#include "ss_signal_types.h"

SsController *ss_controller_new();
void _ss_controller_close_app(SsController *self, gpointer data);
void _ss_controller_backend_stack(SsController *self, gpointer data);
void _ss_controller_backend_notify(SsController *self, gpointer data);

#endif // SS_SIGNAL_H