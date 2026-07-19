#include <gtk/gtk.h>

#include "ss_header.h"
#include "ss_ui_error.h"
#include "ss_header_settings.h"

#include "../core/ss_yaml_types.h"
#include "../infra/utils/ss_utils.h"
#include "../controller/ss_controller.h"


SsHSettingsContext *
ss_hsettings_context_new()
{
    SsHSettingsContext *ctx = g_malloc0(sizeof(SsHSettingsContext));
    if (!ctx)
    {
        ss_send_notify_parse(SS_ERROR_FATAL,
            "não foi possivel alocar memória para o contexto de configurações");
    }
    return ctx;
}


void
ss_hsettings_context_free(SsHSettingsContext **ctx)
{
    if (ctx && *ctx)
    {
        g_free(*ctx);
        *ctx = NULL;
    }
}


static void
toggle_viewer_settings(GtkButton *button, gpointer data)
{
    (void) button;

    SsHSettingsContext *popover = (SsHSettingsContext *) data;
    if (gtk_widget_get_visible(popover->popover_settings))
    {
        gtk_widget_hide(popover->popover_settings);
    }
    else
    {
        gtk_widget_show_all(popover->popover_settings);
    }
}


static void
on_switch_changed(GObject *switch_widget, GParamSpec *pspec, gpointer data)
{
    (void) pspec;

    FlagAndConfig *ptr = (FlagAndConfig *) data;
    bool *state = (ptr->flag == FlagSettingIp ? &ptr->sh_ip.to_use : &ptr->sh_power.to_use);
    *state = gtk_switch_get_active(GTK_SWITCH(switch_widget));
}


static void
updat_can_tooltip_text_in_frame(GtkWidget *box, FlagAndConfig *data)
{
    static char *fmt = "\
\t%s: %x\n\
\t%s: %x\n\
\t%s: %s\n";
    char buf[512];
    snprintf(buf, SS_AS(buf), fmt,
             ss_subtag_yaml_to_string(data->identifier, SS_SUBTAG_YAML_CAN_FRAME), data->sh_power.msg.data[0],
             ss_subtag_yaml_to_string(data->identifier, SS_SUBTAG_YAML_CAN_MESSAGE), data->sh_power.msg.can_id,
             ss_subtag_yaml_to_string(data->identifier, SS_SUBTAG_YAML_CAN_INTERFACE), data->sh_power.can_interface);
    gtk_widget_set_tooltip_text(box, buf);
}


static void
updat_sensor_tooltip_text_in_frame(GtkWidget *box, FlagAndConfig *data)
{
    static char *fmt = "\
\t%s: %s\n\
\t%s: %s\n\
\t%s: %d\n";
    char buf[512];
    snprintf(buf, SS_AS(buf), fmt,
             ss_subtag_yaml_to_string(SS_TAG_YAML_SENSORS, SS_SUBTAG_YAML_IP_ID), data->sh_ip.sensor,
             ss_subtag_yaml_to_string(SS_TAG_YAML_SENSORS, SS_SUBTAG_YAML_IP_IP), data->sh_ip.ip,
             ss_subtag_yaml_to_string(SS_TAG_YAML_SENSORS, SS_SUBTAG_YAML_IP_CRITICAL_LEVEL), data->sh_ip.critical_level);
    gtk_widget_set_tooltip_text(box, buf);
}


static void
edit_setting(GtkButton *button, gpointer data)
{
    FlagAndConfig *setting = (FlagAndConfig *) data;
    
}


static void
create_new_frame_settings(int line, GtkWidget *grid, bool to_use,
    const char *box_name, FlagAndConfig *data)
{
    static size_t len;
    static const char *tag_name = NULL;

    if (__builtin_expect(!tag_name, false))
    {
        tag_name = ss_tag_yaml_to_string(SS_TAG_YAML_SENSORS);
        len = strlen(tag_name);
    }

    const char *lbox_name = box_name;
    GtkWidget *label = NULL;

    if (strncmp(lbox_name, tag_name, len) == 0)
    {
        char name[len + 3];
        snprintf(name, sizeof(name), "%.*s%d", (int) (len - 1), box_name, line);

        label = gtk_label_new(name);
        updat_sensor_tooltip_text_in_frame(label, data);
    }
    else
    {
        label = gtk_label_new(lbox_name);
        updat_can_tooltip_text_in_frame(label, data);
    }

    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_hexpand(label, TRUE);
    gtk_widget_set_margin_start(label, 40);
    gtk_grid_attach(GTK_GRID(grid), label, 0, line, 1, 1);

    GtkWidget *sw = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(sw), to_use);
    g_signal_connect(sw, "notify::active", G_CALLBACK(on_switch_changed), (gpointer) data);

    gtk_grid_attach(GTK_GRID(grid), sw, 1, line, 1, 1);

    GtkWidget *config = gtk_button_new_from_icon_name("preferences-system", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(config, "Alterar");
    g_signal_connect(config, "clicked", G_CALLBACK(edit_setting), (gpointer) data);

    gtk_widget_set_margin_end(config, 10);
    gtk_grid_attach(GTK_GRID(grid), config, 2, line, 1, 1);
}


static void
toggle_viewer_buttons_settings(GtkButton *button, gpointer data)
{
    GtkWidget *dropdown = (GtkWidget *) data;
    GtkWidget *img = gtk_button_get_image(button);

    if (gtk_widget_get_visible(dropdown))
    {
        gtk_widget_hide(dropdown);
        gtk_image_set_from_icon_name(GTK_IMAGE(img), "pan-up-symbolic", GTK_ICON_SIZE_BUTTON);
    }
    else
    {
        gtk_widget_show(dropdown);
        gtk_image_set_from_icon_name(GTK_IMAGE(img), "pan-down-symbolic", GTK_ICON_SIZE_BUTTON);
    }
}


static void
create_settings(GtkWidget *box_settings, const char *title, SsSettingsClass *setting)
{
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(box_settings), header_box, FALSE, FALSE, 0);
    gtk_widget_set_valign(header_box, GTK_ALIGN_START);
    gtk_widget_set_halign(header_box, GTK_ALIGN_START);

    setting->label.widget = gtk_label_new(title);
    gtk_widget_set_halign(setting->label.widget, GTK_ALIGN_START);
    gtk_widget_set_margin_start(setting->label.widget, 20);
    gtk_box_pack_start(GTK_BOX(header_box), setting->label.widget, TRUE, TRUE, 0);

    GtkWidget *line = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(box_settings), line, FALSE, FALSE, 0);

    GtkWidget *config = gtk_button_new_from_icon_name("pan-down-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(header_box), config, FALSE, FALSE, 0);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(box_settings), box, FALSE, FALSE, 0);

    SsWidgetStyleContext edge_box = {.style = SS_STYLE_EG_WHITE, .widget = box};
    ss_update_style_widget(&edge_box);

    g_signal_connect(config, "clicked", G_CALLBACK(toggle_viewer_buttons_settings), (gpointer) box);

    setting->grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(setting->grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(setting->grid), 20);

    gtk_box_pack_start(GTK_BOX(box), setting->grid, TRUE, TRUE, 10);
}


static void
create_cans_settings(GtkWidget *box_settings, const char *name_field, SsSettingsClass *conf_fields)
{
    create_settings(box_settings, name_field, conf_fields);
    ss_configs_t configs;

    ss_controller_get_configs(&configs);

    for (short i = 0; i < conf_fields->size; i++)
    {
        conf_fields->field_forms[i].conf.flag = FlagSettingCan;
        conf_fields->field_forms[i].conf.identifier = i;
        conf_fields->field_forms[i].conf.sh_power = configs.power[i];
        create_new_frame_settings(i, conf_fields->grid, configs.power[i].to_use, ss_tag_yaml_to_string(i),
                                    &conf_fields->field_forms[i].conf);
    }
}


static void
create_sensors_settings(GtkWidget *box_settings, const char *name_field, SsSettingsClass *conf_fields)
{
    create_settings(box_settings, name_field, conf_fields);
    ss_configs_t configs;

    ss_controller_get_configs(&configs);

    for (short i = 0; i < conf_fields->size; i++)
    {
        conf_fields->field_forms[i].conf.flag = FlagSettingIp;
        conf_fields->field_forms[i].conf.identifier = i;
        conf_fields->field_forms[i].conf.sh_ip = configs.sensors.ips[i];
        create_new_frame_settings(i, conf_fields->grid, configs.sensors.ips[i].to_use,
            ss_tag_yaml_to_string(SS_TAG_YAML_SENSORS), &conf_fields->field_forms[i].conf);
    }
}


static void
add_new_sensor(GtkButton *button, gpointer data)
{
    SsSettingsClass *setting = (SsSettingsClass *) data;
    
}


void
ss_create_box_settings(SsHSettingsContext *ctx, GtkWidget *button, const SsGeometryWindow *geo_win)
{
    ctx = ss_hsettings_context_new();
    ctx->button = button;

    //base
    ctx->popover_settings = gtk_popover_new(button);
    gtk_popover_set_position(GTK_POPOVER(ctx->popover_settings), GTK_POS_BOTTOM);
    gtk_widget_set_size_request(ctx->popover_settings, geo_win->width*0.6, geo_win->height*0.99);
    g_signal_connect(button, "clicked", G_CALLBACK(toggle_viewer_settings), ctx);

    GtkWidget *scroll_settings = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(ctx->popover_settings), scroll_settings);

    ctx->overlay_settings = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(scroll_settings), ctx->overlay_settings);

    ctx->box_settings = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(ctx->overlay_settings), ctx->box_settings);

    //header
    GtkWidget *header_settings = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(ctx->box_settings), header_settings, FALSE, FALSE, 10);

    GtkWidget *title = gtk_label_new("Settings");
    gtk_box_pack_start(GTK_BOX(header_settings), title, TRUE, TRUE, 10);

    GtkWidget *new_sensor = gtk_button_new_with_label("New sensor");
    gtk_box_pack_start(GTK_BOX(header_settings), new_sensor, FALSE, FALSE, 15);
    gtk_widget_set_valign(new_sensor, GTK_ALIGN_END);

    g_signal_connect(new_sensor, "clicked", G_CALLBACK(add_new_sensor), (gpointer) &ctx->list_configs[FlagSettingIp]);

    //body
    GtkWidget *body_settings = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(ctx->box_settings), body_settings, FALSE, FALSE, 0);
    gtk_widget_set_valign(new_sensor, GTK_ALIGN_CENTER);

    ctx->list_configs[FlagSettingCan].size = SS_POWER_COUNT;
    ctx->list_configs[FlagSettingCan].field_forms = (SsFieldForms *) g_malloc0(sizeof(SsFieldForms) * SS_POWER_COUNT);
    ss_test_alloc(ctx->list_configs[FlagSettingCan].field_forms);
    create_cans_settings(ctx->box_settings, "Power", &ctx->list_configs[FlagSettingCan]);

    ss_configs_t configs = {0};
    ss_controller_get_configs(&configs);
    ctx->list_configs[FlagSettingIp].size = configs.sensors.size;
    ctx->list_configs[FlagSettingIp].field_forms = (SsFieldForms *) g_malloc0(sizeof(SsFieldForms) * configs.sensors.size);
    ss_test_alloc(ctx->list_configs[FlagSettingIp].field_forms);
    create_sensors_settings(ctx->box_settings, "Sensors", &ctx->list_configs[FlagSettingIp]);
}