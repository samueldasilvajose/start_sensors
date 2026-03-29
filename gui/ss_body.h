#ifndef SS_FOOTER_H
#define SS_FOOTER_H

#include "ss_main.h"
#include "ss_types.h"
#include "ss_style.h"

#define IMG_SENSORS_ON  SS_PATH_GUI"/img/power_sensors.png"
#define IMG_SENSORS_OFF  SS_PATH_GUI"/img/poweroff_sensors.png"

typedef enum
{
    SS_THEME_POWERON,
    SS_THEME_POWEROFF,
    SS_THEME_COUNT
} SsThemeType;

typedef struct _SsBodyContext
{
    SsTheme body[SS_THEME_COUNT];
    SsThemeType current_theme;
} SsBodyContext;

SsBodyContext *ss_body_context_new();
void ss_body_context_free(SsBodyContext **ctx);

void ss_create_body(SsBodyContext **body_ctx, SsMainWindowContext *win_ctx);
void ss_create_footer(SsBodyContext *body_ctx, SsMainWindowContext *win_ctx);
int ss_set_power_theme(SsBodyContext *ctx, int theme, const char *msg);

#endif // SS_FOOTER_H