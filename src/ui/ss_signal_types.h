#ifndef SS_SIGNAL_TYPES_H
#define SS_SIGNAL_TYPES_H

#include <glib-2.0/glib-object.h>

// Macro para o tipo Controller
#define TYPE_SS_CONTROLLER (ss_controller_get_type())
G_DECLARE_FINAL_TYPE(SsController, ss_controller, , SS_CONTROLLER, GObject)

#define SS_SIGNAL_UPDATE_BACKEND_LIST(X) \
    X(STACK,              "SsStack")            \
    X(NOTIFICATION,       "SsNotification")     \
    X(CLOSE_APPLICATION,  "SsCloseApplication")


typedef enum
{
#define X(name, str) SS_SIGNAL_UPDATE_BACKEND_##name,
    SS_SIGNAL_UPDATE_BACKEND_LIST(X)
#undef X
    SS_SIGNAL_UPDATE_BACKEND_COUNT
} SsSignal;

static inline const char *
ss_singnal_to_string(SsSignal type)
{
    static const char *_ss_signal_strings[] =
{
#define X(name, str) str,
    SS_SIGNAL_UPDATE_BACKEND_LIST(X)
#undef X
};

    if ((unsigned) type >= SS_SIGNAL_UPDATE_BACKEND_COUNT)
    {
        return "";
    }
    return _ss_signal_strings[type];
}

#endif // SS_SIGNAL_TYPES_H