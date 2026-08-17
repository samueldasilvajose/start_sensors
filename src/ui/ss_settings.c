#include <gtk/gtk.h>

#include "ss_ui_error.h"
#include "ss_settings.h"

#define USE_YAML_SUBTAG_SETTERS
#include "../core/ss_yaml_types.h"
#undef USE_YAML_SUBTAG_SETTERS

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
on_switch_changed(GObject *switch_widget, GParamSpec *pspec, gpointer data)
{
    (void) pspec;

    FlagAndConfig *ptr = (FlagAndConfig *) data;

    bool flag =(ptr->flag == FlagSettingIp);
    bool *state = (flag ? &ptr->sh_ip.to_use : &ptr->sh_power.to_use);

    *state = gtk_switch_get_active(GTK_SWITCH(switch_widget));

    (flag) ? ss_controller_edit_ip(ptr->identifier, &ptr->sh_ip) :
        ss_controller_edit_can(ptr->identifier, &ptr->sh_power);
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
update_configs(GtkWidget *button, gpointer data)
{
    static size_t len;
    static const char *tag_name = NULL;

    (void) button;

    SsFieldForms *config = data;

    bool falg = (config->conf.flag == FlagSettingIp);
    on_switch_changed(G_OBJECT(config->forms.select->sw), NULL, &config->conf);

    for (int i = 0; i < config->forms.insert_size - 1; i++)
    {
        const char *buf = gtk_entry_get_text(GTK_ENTRY(config->forms.insert[i].entry));
        (falg) ? ss_set_str_subtag_yaml_ip(&config->conf.sh_ip, i + 1, buf) :
             ss_set_str_subtag_yaml_can(&config->conf.sh_power, i + 1, buf);
    }

    if (falg)
    {
        if (!tag_name)
        {
            tag_name = ss_tag_yaml_to_string(SS_TAG_YAML_SENSORS);
            len = strlen(tag_name);
        }

        char name[len + 3];
        snprintf(name, sizeof(name), "%.*s%d", (int) (len - 1), tag_name, config->conf.identifier);
        ss_send_notify_parse(SS_ERROR_WARNING,
            "configuração do %s (%s) alterada.", name, config->conf.sh_ip.sensor);
        ss_controller_edit_ip(config->conf.identifier, &config->conf.sh_ip);
    }
    else
    {
        ss_send_notify_parse(SS_ERROR_WARNING,
            "configuração da comunicação can (%s) alterada.", ss_tag_yaml_to_string(config->conf.identifier));
        ss_controller_edit_can(config->conf.identifier, &config->conf.sh_power);
    }
}


static void
close_edit_setting(GtkButton *button, gpointer data)
{
    (void) button;
    GtkWidget *popover = data;

    if (gtk_widget_get_visible(popover))
        gtk_widget_hide(popover);
}


static void
creat_edit_setting(GtkWidget *button, bool to_use, SsFieldForms *data, const SsGeometryWindow *geo)
{
    data->forms.popover = gtk_popover_new(button);
    gtk_popover_set_position(GTK_POPOVER(data->forms.popover), GTK_POS_BOTTOM);
    gtk_widget_set_size_request(data->forms.popover, geo->width, geo->height);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(data->forms.popover), scroll);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(scroll), box);

    data->forms.grid = gtk_grid_new();
    gtk_box_pack_start(GTK_BOX(box), data->forms.grid, TRUE, TRUE, 15);
    gtk_grid_set_row_spacing(GTK_GRID(data->forms.grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(data->forms.grid), 10);

    int config_disp = (data->conf.flag == FlagSettingCan) ? data->conf.identifier : SS_TAG_YAML_SENSORS;

    data->forms.insert_size = ss_get_qtd_subtags_yaml(config_disp);
    data->forms.insert = g_malloc0(sizeof(SsInsertForms) * (data->forms.insert_size - 1));
    ss_test_alloc(data->forms.insert);

    data->forms.select_size = 1;
    data->forms.select = g_malloc0(sizeof(SsSelectForms) * data->forms.select_size);
    ss_test_alloc(data->forms.select);

    data->forms.select->sw = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(data->forms.select->sw), to_use);
    gtk_widget_set_hexpand(data->forms.select->sw, FALSE);

    data->forms.select->label.style = SS_STYLE_WHITE;
    data->forms.select->label.widget = gtk_label_new(ss_subtag_yaml_to_string(config_disp, 0));
    ss_setup_form_label(data->forms.select->label.widget);

    gtk_grid_attach(GTK_GRID(data->forms.grid), data->forms.select->label.widget, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(data->forms.grid), data->forms.select->sw, 1, 0, 1, 1);
    gtk_widget_set_halign(data->forms.select->sw, GTK_ALIGN_START);

    SsInsertForms *insert = data->forms.insert;
    for (int i = 0; i < data->forms.insert_size - 1; i++)
    {
        insert[i].entry = gtk_entry_new();
        gtk_widget_set_hexpand(insert[i].entry, FALSE);

        insert[i].label.style = SS_STYLE_WHITE;
        insert[i].label.widget = gtk_label_new(ss_subtag_yaml_to_string(config_disp, i + 1));
        ss_setup_form_label(insert[i].label.widget);

        gtk_grid_attach(GTK_GRID(data->forms.grid), insert[i].label.widget, 0, i + 1, 1, 1);
        gtk_grid_attach(GTK_GRID(data->forms.grid), insert[i].entry, 1, i + 1, 1, 1);
    }

    GtkWidget *box_button = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(box), box_button, FALSE, FALSE, 20);
    gtk_widget_set_halign(box_button, GTK_ALIGN_CENTER);

    GtkWidget *save_config = gtk_button_new_with_label("save");
    gtk_box_pack_start(GTK_BOX(box_button), save_config, FALSE, FALSE, 0);
    gtk_widget_set_halign(save_config, GTK_ALIGN_CENTER);

    g_signal_connect(save_config, "clicked", G_CALLBACK(update_configs), (gpointer) data);

    GtkWidget *close = gtk_button_new_with_label("close");
    gtk_box_pack_start(GTK_BOX(box_button), close, FALSE, FALSE, 0);

    g_signal_connect(close, "clicked", G_CALLBACK(close_edit_setting), data->forms.popover);
}


static void
edit_setting(GtkButton *button, gpointer data)
{
    (void) button;
    SsFieldForms *forms = (SsFieldForms *) data;

    if (gtk_widget_get_visible(forms->forms.popover))
    {
        gtk_widget_hide(forms->forms.popover);
        return;
    }

    char buf[512] = {0};
    for (int i = 0; i < forms->forms.insert_size - 1; i++)
    {
        bool success;
        success = (forms->conf.flag == FlagSettingIp) ? ss_get_str_subtag_yaml_ip(&forms->conf.sh_ip, i + 1, buf, sizeof(buf)) :
            ss_get_str_subtag_yaml_can(&forms->conf.sh_power, i + 1, buf, sizeof(buf));

        if (success)
            gtk_entry_set_text(GTK_ENTRY(forms->forms.insert[i].entry), buf);
    }
    
    gtk_widget_show_all(forms->forms.popover);
}


static gboolean
on_label_enter(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
    (void) event;

    GtkWidget *label = gtk_bin_get_child(GTK_BIN(widget));
    FlagAndConfig *config = data;

    (config->flag == FlagSettingIp) ? updat_sensor_tooltip_text_in_frame(label, config) :
        updat_can_tooltip_text_in_frame(label, config);
    return FALSE;
}


static void
create_new_frame_settings(int line, GtkWidget *grid, bool to_use,
    const char *box_name, SsFieldForms *data, const SsGeometryWindow *geo)
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
        updat_sensor_tooltip_text_in_frame(label, &data->conf);
    }
    else
    {
        label = gtk_label_new(lbox_name);
        updat_can_tooltip_text_in_frame(label, &data->conf);
    }

    GtkWidget *event_box = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(event_box), label);
    gtk_widget_add_events(event_box, GDK_ENTER_NOTIFY_MASK);
    g_signal_connect(event_box, "enter-notify-event", G_CALLBACK(on_label_enter), &data->conf);

    gtk_widget_set_halign(event_box, GTK_ALIGN_START);
    gtk_widget_set_hexpand(event_box, TRUE);
    gtk_widget_set_margin_start(event_box, 40);
    gtk_grid_attach(GTK_GRID(grid), event_box, 0, line, 1, 1);

    GtkWidget *sw = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(sw), to_use);
    g_signal_connect(sw, "notify::active", G_CALLBACK(on_switch_changed), (gpointer) &data->conf);

    gtk_grid_attach(GTK_GRID(grid), sw, 1, line, 1, 1);

    GtkWidget *config = gtk_button_new_from_icon_name("preferences-system", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(config, "Alterar");

    creat_edit_setting(config, to_use, data, geo);
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
remove_sensor(GtkButton *button, gpointer data)
{
    (void) button;
    (void) data;
}


static void
add_new_sensor(GtkButton *button, gpointer data)
{
    (void) button;
    (void) data;
}


static void
create_settings(GtkWidget *box_settings, const char *title, SsSettingsClass *setting)
{
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(box_settings), header_box, TRUE, TRUE, 20);
    gtk_widget_set_valign(header_box, GTK_ALIGN_START);
    gtk_widget_set_halign(header_box, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(header_box, TRUE);

    setting->label.widget = gtk_label_new(title);
    gtk_widget_set_halign(setting->label.widget, GTK_ALIGN_START);
    gtk_widget_set_margin_start(setting->label.widget, 20);
    gtk_box_pack_start(GTK_BOX(header_box), setting->label.widget, FALSE, FALSE, 0);

    GtkWidget *line = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(box_settings), line, FALSE, FALSE, 0);

    GtkWidget *config = gtk_button_new_from_icon_name("pan-down-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(header_box), config, FALSE, FALSE, 0);

    if (0) //(setting->field_forms->conf.flag == FlagSettingIp)
    {
        // header
        GtkWidget *header_settings = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_box_pack_end(GTK_BOX(header_box), header_settings, FALSE, FALSE, 10);

        GtkWidget *new_sensor = gtk_button_new_with_label("New sensor");
        gtk_box_pack_start(GTK_BOX(header_settings), new_sensor, FALSE, FALSE, 0);
        g_signal_connect(new_sensor, "clicked", G_CALLBACK(add_new_sensor), (gpointer) setting);

        GtkWidget *delete_sensor = gtk_button_new_with_label("Delete sensor");
        gtk_box_pack_start(GTK_BOX(header_settings), delete_sensor, FALSE, FALSE, 0);
        g_signal_connect(delete_sensor, "clicked", G_CALLBACK(remove_sensor), (gpointer) setting);
    }

    GtkWidget *scroll_settings = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(box_settings), scroll_settings);
    gtk_widget_set_size_request(scroll_settings, -1, 45*4);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(scroll_settings), box);

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
                                    &conf_fields->field_forms[i], &conf_fields->geo);
    }
}


static void
create_sensors_settings(GtkWidget *box_settings, const char *name_field, SsSettingsClass *conf_fields)
{
    ss_configs_t configs;
    ss_controller_get_configs(&configs);

    create_settings(box_settings, name_field, conf_fields);

    for (short i = 0; i < conf_fields->size; i++)
    {
        conf_fields->field_forms[i].conf.flag = FlagSettingIp;
        conf_fields->field_forms[i].conf.identifier = i;
        conf_fields->field_forms[i].conf.sh_ip = configs.sensors.ips[i];
        create_new_frame_settings(i, conf_fields->grid, configs.sensors.ips[i].to_use,
            ss_tag_yaml_to_string(SS_TAG_YAML_SENSORS), &conf_fields->field_forms[i], &conf_fields->geo);
    }
}


void
ss_create_settings(SsHSettingsContext **ctx, SsMainWindowContext *win_ctx)
{
    SsHSettingsContext *st_ctx = ss_hsettings_context_new();
    *ctx = st_ctx;

    const char *np = ss_get_name_page(SS_NAME_PAGE_SETTINGS);

    GtkWidget *label_np = gtk_label_new(np);
    gtk_list_box_insert(GTK_LIST_BOX(win_ctx->sidebar->list), label_np, SS_NAME_PAGE_SETTINGS);

    st_ctx->page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_stack_add_named(GTK_STACK(win_ctx->sidebar->stack),
                            st_ctx->page, np);

    //body
    GtkWidget *body_settings = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(st_ctx->page), body_settings, FALSE, FALSE, 10);

    SsGeometryWindow geo = win_ctx->geo;
    geo.height *= 0.7; geo.width *= 0.4;

    st_ctx->list_configs[FlagSettingCan].geo = geo;
    st_ctx->list_configs[FlagSettingCan].size = SS_POWER_COUNT;
    st_ctx->list_configs[FlagSettingCan].field_forms = (SsFieldForms *) g_malloc0(sizeof(SsFieldForms) * SS_POWER_COUNT);
    ss_test_alloc(st_ctx->list_configs[FlagSettingCan].field_forms);

    st_ctx->list_configs[FlagSettingCan].field_forms->conf.flag = FlagSettingCan;
    create_cans_settings(body_settings, "Power", &st_ctx->list_configs[FlagSettingCan]);

    ss_configs_t configs = {0};
    ss_controller_get_configs(&configs);
    st_ctx->list_configs[FlagSettingIp].geo = geo;
    st_ctx->list_configs[FlagSettingIp].size = configs.sensors.size;
    st_ctx->list_configs[FlagSettingIp].field_forms = (SsFieldForms *) g_malloc0(sizeof(SsFieldForms) * configs.sensors.size);
    ss_test_alloc(st_ctx->list_configs[FlagSettingIp].field_forms);

    st_ctx->list_configs[FlagSettingIp].field_forms->conf.flag = FlagSettingIp;
    create_sensors_settings(body_settings, "Sensors", &st_ctx->list_configs[FlagSettingIp]);
}
