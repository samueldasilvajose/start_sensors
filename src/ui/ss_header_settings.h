#ifndef SS_HEADER_SETTINGS_H
#define SS_HEADER_SETTINGS_H

#include "ss_types.h"


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
    GtkWidget *popover_settings;
    SsSettingsClass *list_configs;
} SsHSettingsContext;


SsHSettingsContext *ss_hsettings_context_new();
void ss_hsettings_context_free(SsHSettingsContext **ctx);

#endif //SS_HEADER_SETTINGS_H