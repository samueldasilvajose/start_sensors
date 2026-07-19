#include "ss_ui_error.h"
#include "ss_dashboard.h"
#include "../controller/ss_controller.h"


SsDashboardContext *
ss_dashboard_context_new()
{
    return g_malloc0(sizeof(SsDashboardContext));
}


void
ss_dashboard_context_free(SsDashboardContext **ctx)
{
    if (ctx && *ctx)
    {
        g_free(*ctx);
        *ctx = NULL;
    }
}


//stack
void
ss_define_item_in_stack(SsController *ctl, gpointer data, gpointer user_data)
{
    (void) ctl;

    SsStackComponentsType type = *((SsStackComponentsType *) data);
    SsStackContext *ctx = (SsStackContext *) user_data;
    gtk_stack_set_visible_child_name(GTK_STACK(ctx->stack), ctx->components[type].label);
}


static void
set_items_in_stack(SsStackContext *data)
{
    static const char *name_img[SS_STACK_COMPONENTS_COUNT] = \
    {
        [SS_STACK_COMPONENTS_OK] = "dialog-ok",
        [SS_STACK_COMPONENTS_ERROR] = "process-stop", 
        [SS_STACK_COMPONENTS_EMPTY] = "dialog-question", 
        [SS_STACK_COMPONENTS_WARNING] = "dialog-warning", 
        [SS_STACK_COMPONENTS_LOADING] = NULL, 
    };

    for (int i = 0; i < SS_STACK_COMPONENTS_COUNT; i++)
    {
        const char *aux = name_img[i];
        if (aux == NULL)
        {
           continue;
        }
        
        SsStackComponent *component = &data->components[i];
        component->img = gtk_image_new_from_icon_name(aux, GTK_ICON_SIZE_LARGE_TOOLBAR);
        component->label = ss_stack_components_to_string(i);
        gtk_stack_add_named(GTK_STACK(data->stack), component->img, component->label);
    }

    SsStackComponent *component = &data->components[SS_STACK_COMPONENTS_LOADING];
    component->img = gtk_spinner_new();
    component->label = ss_stack_components_to_string(SS_STACK_COMPONENTS_LOADING);
    gtk_spinner_start(GTK_SPINNER(component->img));
    gtk_widget_set_size_request(component->img, 30, 30);
    gtk_stack_add_named(GTK_STACK(data->stack), component->img, component->label);
}


void
ss_create_stack(SsDashboardContext *header_ctx, SsMainWindowContext *win_ctx)
{
    SsStackContext *stack = &header_ctx->stack;

    stack->stack = gtk_stack_new();
    gtk_box_pack_start(GTK_BOX(header_ctx->page), stack->stack, TRUE, TRUE, 10);
    gtk_widget_set_halign(stack->stack, GTK_ALIGN_START);  // direita
    gtk_widget_set_margin_start(stack->stack, 10);
    gtk_widget_set_tooltip_text(stack->stack, "Status dos sensores");

    set_items_in_stack(stack);
    g_signal_connect (win_ctx->controller, ss_singnal_to_string(SS_SIGNAL_UPDATE_BACKEND_STACK),
        G_CALLBACK(ss_define_item_in_stack), stack);
}


int
ss_set_power_theme(SsDashboardContext *ctx, int theme, const char *msg)
{
    SsTheme *t = &ctx->body[theme];
    if ((SsThemeType) theme == ctx->current_theme)
        return 1;

    t->label._first_style = ctx->body[ctx->current_theme].label.style;
    t->frame._first_style = ctx->body[ctx->current_theme].frame._first_style;
    
    ss_apply_theme(t);
    gtk_label_set_text(GTK_LABEL(t->label.widget), msg);
    ctx->current_theme = theme;

    return 0;
}


static void
action_poweron(SsDashboardContext *ctx)
{
    ss_check_ips();
    ss_set_power_theme(ctx, SS_THEME_POWERON, "Sensores ligados!");
}

static void
poweron_sensors(GtkWidget *button, gpointer data)
{
    (void) button;

    SsDashboardContext *ctx = data;
    if (SS_THEME_POWERON == ctx->current_theme)
        return;

    ss_poweron_sensors();
    ss_set_power_theme(ctx, SS_THEME_POWERON, "Sensores ligados!");
}


static void
action_poweroff(SsDashboardContext *ctx)
{
    ss_set_power_theme(ctx, SS_THEME_POWEROFF, "Sensores desligados!");
    ss_check_ips_encerrer();
}

static void
poweroff_sensors(GtkWidget *button, gpointer data)
{
    (void) button;
    
    SsDashboardContext *ctx = data;
    if (SS_THEME_POWEROFF == ctx->current_theme)
        return;

    ss_poweroff_sensors();
    ss_set_power_theme(ctx, SS_THEME_POWEROFF, "Sensores desligados!");
}


void
set_default_state(SsDashboardContext *ctx)
{
    ss_send_stack_msg(SS_STACK_COMPONENTS_EMPTY);
    ss_set_power_theme(ctx, SS_THEME_POWEROFF, "");
}


static void
desired_power_status(SsDashboardContext *ctx)
{
    static int previous_state = -2;

    int current_state = ss_read_state_sensors();
    if (previous_state == current_state)
        return;

    if (current_state >= 0)
    {
        (current_state) ? action_poweron(ctx) : action_poweroff(ctx);
        previous_state = current_state;
    }
    else
    {
        set_default_state(ctx);
    }
}


gboolean
ss_desired_power_status(gpointer data)
{
    SsDashboardContext *ctx = (SsDashboardContext *) data;
    desired_power_status(ctx);
    
    return FALSE;
}


void
ss_create_dashboard(SsDashboardContext **dashboard_ctx, SsMainWindowContext *win_ctx)
{
    SsDashboardContext *db_ctx = ss_dashboard_context_new();
    *dashboard_ctx = db_ctx;

    const char *np = ss_get_name_page(SS_NAME_PAGE_DASHBOARD);

    GtkWidget *label_np = gtk_label_new(np);
    gtk_list_box_insert(GTK_LIST_BOX(win_ctx->sidebar->list), label_np, SS_NAME_PAGE_DASHBOARD);

    db_ctx->page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_stack_add_named(GTK_STACK(win_ctx->sidebar->stack),
                            db_ctx->page, np);

    SsTheme *on = &db_ctx->body[SS_THEME_POWERON];
    on->frame.style = SS_STYLE_BG_WHITE;
    on->frame._first_style = SS_STYLE_NONE;
    on->frame.widget = db_ctx->page;
    on->wallpaper.wallpaper_path = IMG_SENSORS_ON;
    on->label.style = SS_STYLE_GREEN;

    SsTheme *off = &db_ctx->body[SS_THEME_POWEROFF];
    db_ctx->current_theme = SS_THEME_COUNT;
    off->frame.style = SS_STYLE_BG_GRAY;
    off->frame._first_style = SS_STYLE_NONE;
    off->frame.widget = db_ctx->page;
    off->wallpaper.wallpaper_path = IMG_SENSORS_OFF;
    off->label.style = SS_STYLE_RED;

    GtkWidget *img = gtk_image_new();
    on->wallpaper.img = off->wallpaper.img = img;
    on->wallpaper.wallpaper_geometry = off->wallpaper.wallpaper_geometry = win_ctx->geo;
    gtk_widget_set_size_request(img, win_ctx->geo.width*0.1, win_ctx->geo.height);

    GtkWidget *lebal_status = gtk_label_new("");
    gtk_widget_set_halign(lebal_status, GTK_ALIGN_CENTER);

    gtk_widget_set_margin_bottom(lebal_status, 30);
    on->label.widget = off->label.widget = lebal_status;
    
    ss_create_stack(db_ctx, win_ctx);

    gtk_box_pack_start(GTK_BOX(db_ctx->page), img, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(db_ctx->page), lebal_status, FALSE, FALSE, 0);

    GtkWidget *footer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(db_ctx->page), footer, FALSE, FALSE, 20);
    gtk_widget_set_halign(footer, GTK_ALIGN_CENTER);

    // Botão ligar
    GtkWidget *but_power_sensors = gtk_button_new_with_label("power_on");
    g_signal_connect(but_power_sensors, "clicked", G_CALLBACK(poweron_sensors), (gpointer) db_ctx);

    // Botão desligar
    GtkWidget *but_poweroff_sensors = gtk_button_new_with_label("power_off");
    g_signal_connect(but_poweroff_sensors, "clicked", G_CALLBACK(poweroff_sensors), (gpointer) db_ctx);

    // Adiciona os botões no footer
    gtk_box_pack_start(GTK_BOX(footer), but_power_sensors, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(footer), but_poweroff_sensors, FALSE, FALSE, 10);

    // Centraliza verticalmente os botões dentro do box_footer
    gtk_widget_set_valign(but_power_sensors, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(but_poweroff_sensors, GTK_ALIGN_CENTER);
}
