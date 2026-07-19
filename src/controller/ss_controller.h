#ifndef SS_CONTROLLER_H
#define SS_CONTROLLER_H

#include "../backend/ss_core.h"
#include "../ui/ss_signal_types.h"

// ================== frontend -> backend ================== //

typedef enum
{
    SS_COMMAND_THREAD_REPEAT,
    SS_COMMAND_THREAD_CLOSE,
    SS_COMMAND_THREAD_COUNT
} ss_command_thread_t;


void ss_check_ips();
void ss_controller_edit_ip(size_t index, ss_sensor_t *ip);
void ss_controller_edit_can(ss_power_index_t index, ss_power_t *can);
void ss_controller_get_configs(ss_configs_t *data);

int ss_poweron_sensors();
int ss_poweroff_sensors();
int ss_read_state_sensors();

void ss_send_command_thread(GAsyncQueue *queue, ss_command_thread_t cmd);
GAsyncQueue *ss_read_can_thread_start();


// ================== backend -> frontend ================== //
int ss_start(int argc, char **argv);
void ss_send_notify_ts(int critical_level, const char *msg);
void ss_send_notify_parse(int critical_level, const char *fmt, ...);
void ss_send_stack_msg(int err);
void ss_send_stack_msg_ts(int err_user);

// ================== utils ================== //
void ss_check_ips_encerrer();
void ss_lock_check_ips_mutex();
void ss_unlock_check_ips_mutex();

SsController *ss_init_controller();

#endif // SS_CONTROLLER_H
