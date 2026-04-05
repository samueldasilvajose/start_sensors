#ifndef SS_CORE_H
#define SS_CORE_H

#include <stdlib.h>
#include "../core/ss_core_types.h"
#include "../infra/error/ss_error.h"
#include "../infra/error/ss_error_message.h"


void ss_set_fconfigs(const char *file);
void ss_save_fconfigs();
void ss_extract_configs_from_file();

void ss_init_can();
void ss_rm_ip(size_t index);
void ss_edit_ip(size_t index, ss_sensor_t *ip);
void ss_edit_can(ss_power_index_t index, ss_power_t *can);
const ss_configs_t *ss_get_configs();

int ss_check_ip(int index_ip);
int ss_exec_poweron();
int ss_exec_poweroff();
int ss_exec_read_state();
ss_error_mask_t ss_set_core_level_to_notify(ss_error_mask_t level);

#endif //SS_CORE_H