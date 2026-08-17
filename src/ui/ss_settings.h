#ifndef SS_HEADER_SETTINGS_H
#define SS_HEADER_SETTINGS_H

#include "ss_main.h"
#include "ss_style.h"
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
    GtkWidget *entry;
    SsWidgetStyleContext label;
} SsInsertForms;

typedef struct
{
    GtkWidget *sw;
    SsWidgetStyleContext label;
} SsSelectForms;

typedef struct
{
    short insert_size;
    short select_size;
    GtkWidget *grid;
    GtkWidget *popover;
    SsInsertForms *insert;
    SsSelectForms *select;
} SsForms;

typedef struct
{
    SsForms forms;
    FlagAndConfig conf;
    SsWidgetStyleContext label;
} SsFieldForms;

typedef struct
{
    short size;
    GtkWidget *grid;
    SsFieldForms *field_forms;
    SsGeometryWindow geo;
    SsWidgetStyleContext label;
} SsSettingsClass;

typedef struct _SsHSettingsContext
{
    GtkWidget *page;
    GtkWidget *box_sensors;
    SsSettingsClass list_configs[FlagSettingMax];
} SsHSettingsContext;


SsHSettingsContext *ss_hsettings_context_new();
void ss_hsettings_context_free(SsHSettingsContext **ctx);

void ss_create_settings(SsHSettingsContext **ctx, SsMainWindowContext *win_ctx);

#endif //SS_HEADER_SETTINGS_H