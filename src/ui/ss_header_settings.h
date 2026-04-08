#ifndef SS_HEADER_SETTINGS_H
#define SS_HEADER_SETTINGS_H

#include "ss_types.h"

typedef enum
{
    FlagSettingIp = 0,
    FlagSettingCan,
    FlagSettingMax
} SsFlagSetting;

struct FlagAndConfig
{   
    SsFlagSetting flag;
    union
    {
        ss_power_t power;
        ss_sensor_t ip;
    } SHconfig;
};
#define sh_ip SHconfig.ip
#define sh_power SHconfig.power


typedef struct
{
    GtkWidget *entry_config;
    SsWidgetStyleContext label;
} SsFieldForms;

typedef struct
{
    SsWidgetStyleContext label;
    SsFieldForms *field_forms;
} SsSettingsClass;

typedef struct
{
    GtkWidget *button;
    GtkWidget *box_settings;
    GtkWidget *overlay_settings;
    GtkWidget *popover_settings;
    SsSettingsClass *list_configs;
} SsHSettingsContext;


SsHSettingsContext *ss_hsettings_context_new();
void ss_hsettings_context_free(SsHSettingsContext **ctx);

void ss_create_box_settings(SsHSettingsContext *ctx, GtkWidget *button, const SsGeometryWindow *geo_win);

#endif //SS_HEADER_SETTINGS_H