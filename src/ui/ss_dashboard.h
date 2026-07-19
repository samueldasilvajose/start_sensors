#ifndef SS_FOOTER_H
#define SS_FOOTER_H

#include "ss_main.h"
#include "ss_types.h"
#include "ss_style.h"

#define IMG_SENSORS_ON  SS_PATH_GUI"/../../assets/images/power_sensors.png"
#define IMG_SENSORS_OFF  SS_PATH_GUI"/../../assets/images/poweroff_sensors.png"

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

typedef enum
{
    SS_THEME_POWERON,
    SS_THEME_POWEROFF,
    SS_THEME_COUNT
} SsThemeType;

typedef struct _SsDashboardContext
{
    GtkWidget *page;
    SsTheme body[SS_THEME_COUNT];
    SsThemeType current_theme;
    SsStackContext stack;
} SsDashboardContext;

SsDashboardContext *ss_dashboard_context_new();
void ss_dashboard_context_free(SsDashboardContext **ctx);
void set_default_state(SsDashboardContext *ctx);

void ss_create_dashboard(SsDashboardContext **dashboard_ctx, SsMainWindowContext *win_ctx);
int ss_set_power_theme(SsDashboardContext *ctx, int theme, const char *msg);
void ss_define_item_in_stack(SsController *ctl, gpointer data, gpointer user_data);

gboolean ss_desired_power_status(gpointer data);

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

#endif // SS_FOOTER_H