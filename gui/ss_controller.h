#ifndef SS_CONTROLLER_H
#define SS_CONTROLLER_H

#include "ss_error.h"
#include "ss_signal_types.h"


typedef struct
{
    SsErrorLevel err;
    char msg[512];
}SsNotifyMsg;


// ================== frontend -> backend ================== //

void ss_check_ips();
void ss_poweron_sensors();
void ss_poweroff_sensors();
void ss_read_state_sensors();


// ================== backend -> frontend ================== //
int ss_start(int argc, char **argv);
void ss_send_notify(int critical_level, const char *msg);
void ss_send_notify_ts(int critical_level, const char *msg);
void ss_send_stack_msg(int err);
void ss_send_stack_msg_ts(int err_user);

// ================== utils ================== //
void ss_check_ips_encerrer();
void ss_lock_check_ips_mutex();
void ss_unlock_check_ips_mutex();

SsController *ss_init_controller();

#endif // SS_CONTROLLER_H