#ifndef SS_HEADER_SETTINGS_H
#define SS_HEADER_SETTINGS_H

#include "ss_types.h"
#include "../core/ss_core_types.h"

typedef enum
{
    FlagSettingIp = 0,
    FlagSettingCan,
    FlagSettingMax
} SsFlagSetting;

typedef struct
{   
    short identifier;
    SsFlagSetting flag;
    union
    {
        ss_power_t power;
        ss_sensor_t ip;
    } SHconfig;
} FlagAndConfig;
#define sh_ip SHconfig.ip
#define sh_power SHconfig.power


typedef struct
{
    FlagAndConfig conf;
    SsWidgetStyleContext label;
} SsFieldForms;

typedef struct
{
    short size;
    GtkWidget *grid;
    SsFieldForms *field_forms;
    SsWidgetStyleContext label;
} SsSettingsClass;

typedef struct
{
    GtkWidget *button;
    GtkWidget *box_settings;
    GtkWidget *overlay_settings;
    GtkWidget *popover_settings; 
    SsSettingsClass list_configs[FlagSettingMax];
} SsHSettingsContext;


SsHSettingsContext *ss_hsettings_context_new();
void ss_hsettings_context_free(SsHSettingsContext **ctx);

void ss_create_box_settings(SsHSettingsContext *ctx, GtkWidget *button, const SsGeometryWindow *geo_win);

#endif //SS_HEADER_SETTINGS_H