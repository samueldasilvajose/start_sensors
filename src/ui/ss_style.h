#ifndef START_SENSORS_GUI_H
#define START_SENSORS_GUI_H

#include "ss_types.h"

#define SS_STYLE_LIST(X)      \
    X(RED,      "red")           \
    X(BLACK,    "black")         \
    X(GREEN,    "green")         \
    X(BG_GRAY,  "bg_gray")       \
    X(BG_WHITE, "bg_white")         


typedef enum
{
    SS_STYLE_NONE = -1,
#define X(name, str) SS_STYLE_##name,
    SS_STYLE_LIST(X)
#undef X
    SS_STYLE_COUNT
} SsStyleClass;

typedef struct
{
    GtkWidget *widget;
    SsStyleClass style;
    SsStyleClass _first_style;
} SsWidgetStyleContext;

typedef struct
{
    GtkWidget *img;
    const char *wallpaper_path;
    SsGeometryWindow wallpaper_geometry;
} SsWallpaperContext;

typedef struct
{
    SsStyleClass style;
    SsWidgetStyleContext frame;
    SsWidgetStyleContext label;
    SsWallpaperContext wallpaper;
} SsTheme;

void ss_init_style();
void ss_apply_theme(SsTheme *theme);
void set_image_scaled(SsWallpaperContext *ctx);
void ss_update_style_widget(SsWidgetStyleContext *ctx);


static inline const char *
ss_style_to_string(SsStyleClass type)
{
    static const char *_ss_style_strings[] =
{
#define X(name, str) str,
    SS_STYLE_LIST(X)
#undef X
};
    if ((unsigned) type >= SS_STYLE_COUNT)
    {
        return "";
    }
    return _ss_style_strings[type];
}

#endif //START_SENSORS_GUI_H