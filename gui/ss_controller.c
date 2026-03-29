#include "ss_main.h"
#include "ss_signal.h"
#include "ss_header.h"
#include "ss_controller.h"

#include "../start_sensors.h"

static int id_thread = 0;
static GMutex check_ips_mutex;
static SsController *ss_controller = NULL;


// ================== frontend -> backend ================== //
int
ss_poweron_sensors()
{
    return start_sensores_exec_poweron();
}


int
ss_poweroff_sensors()
{
    ss_send_stack_msg(SS_STACK_COMPONENTS_EMPTY);
    return start_sensores_exec_poweroff();
}


int
ss_read_state_sensors()
{
    return start_sensores_exec_read_state();
}


static gpointer
check_ips(gpointer data)
{
    gsize ips_connect = 0;
    gsize list_ips_size = GPOINTER_TO_SIZE(data);
    for (gsize i = 0; i < list_ips_size; i++)
    {
        ips_connect += start_sensors_check_ip(i);
    }

    if (ips_connect == list_ips_size)
    {
        ss_send_stack_msg_ts(SS_STACK_COMPONENTS_OK);
    }
    else if (ips_connect != 0)
    {
        ss_send_stack_msg_ts(SS_STACK_COMPONENTS_WARNING);
    }
    else
    {
        ss_send_stack_msg_ts(SS_STACK_COMPONENTS_ERROR);
    }

    return NULL;
}

static gboolean
wrapper_check_ips(gpointer data)
{
    *(int *) data = 0;

    gsize size = start_sensores_get_configs()->ips_size;
    GThread *thread = g_thread_new("check_ips", check_ips, GSIZE_TO_POINTER(size));
    g_thread_unref(thread);
    return FALSE;
}

void
ss_check_ips_encerrer()
{
    if (id_thread)
    {
        g_source_remove(id_thread);
        id_thread = 0;
    }
}

void
ss_check_ips()
{
    ss_check_ips_encerrer();
    ss_send_stack_msg(SS_STACK_COMPONENTS_LOADING);
    id_thread = g_timeout_add(10000, wrapper_check_ips, (gpointer) &id_thread); //aguarda 10s
}


// ================== backend -> frontend ================== //

int
ss_start(int argc, char **argv)
{
    return (ss_rum_app(argc, argv));
}


void
ss_send_stack_msg(int err_user)
{
    static SsStackComponentsType err = SS_STACK_COMPONENTS_EMPTY;
    
    if ((unsigned) err > SS_STACK_COMPONENTS_COUNT)
    {
        g_mutex_unlock(&check_ips_mutex);
        return;
    }

    err = err_user;
    _ss_controller_backend_stack(ss_controller, &err);
}

static gboolean
send_stack_msg_ts(gpointer data)
{
    ss_send_stack_msg(*(int *) data);
    g_free(data);

    return FALSE;
}

void
ss_send_stack_msg_ts(int err_user)
{
    int *data = g_new(int, 1);
    *data = err_user;

    g_idle_add(send_stack_msg_ts, (gpointer) data);
}


void
ss_send_notify(int critical_level, const char *msg)
{
    static SsNotifyMsg noti = {0};
    
    if ((unsigned) critical_level >= SS_ERROR_COUNT || !msg)
    {
        return;
    }
    
    noti.err = critical_level;
    g_strlcpy(noti.msg, msg, (sizeof(noti.msg)/sizeof(noti.msg[0])));
    start_sensores_salve_log(&noti);

    if (noti.err == SS_ERROR_ABORT)
    {
        ss_set_error(SS_ERROR_ABORT);
        _ss_controller_close_app(ss_controller, NULL);
    }
    
    _ss_controller_backend_notify(ss_controller, &noti);
}

static gboolean
send_notify_ts(gpointer data)
{
    SsNotifyMsg *noti = data;
    ss_send_notify(noti->err, noti->msg);
    g_free(noti);

    return FALSE;
}

void
ss_send_notify_ts(int critical_level, const char *msg)
{
    SsNotifyMsg *data = g_new0(SsNotifyMsg, 1);
    data->err = critical_level;
    g_strlcpy(data->msg, msg, (sizeof(data->msg)/sizeof(data->msg[0])));
    g_idle_add(send_notify_ts, (gpointer) data);
}


// ================== utils ================== //

void
ss_lock_check_ips_mutex()
{
    g_mutex_lock(&check_ips_mutex);
}


void
ss_unlock_check_ips_mutex()
{
    g_mutex_unlock(&check_ips_mutex);
}


SsController *
ss_init_controller()
{
    if (!ss_controller)
    {
        g_mutex_init(&check_ips_mutex);
        ss_controller = ss_controller_new();
    }
    return ss_controller;
}
