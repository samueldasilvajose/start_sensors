#include <gtk/gtk.h>
#include "ss_app_context.h"

SsAppContext *
ss_app_context_new()
{
    return g_malloc0(sizeof(SsAppContext));
}

void
ss_app_context_free(SsAppContext **ctx)
{
    if (ctx && *ctx)
    {
        g_free(*ctx);
        *ctx = NULL;
    }
}
