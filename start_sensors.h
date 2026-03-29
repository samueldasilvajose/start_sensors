#ifndef START_SENSOR_H
#define START_SENSOR_H

#include "start_sensors_types.h"
#include "start_sensors_utils.h"


void start_sensors_init_log(const char *log_error_path);
void start_sensores_salve_log(SsNotifyMsg *noti);
void start_sensores_set_fconfigs(const char *file);
void start_sensores_save_fconfigs();
void start_sensores_extract_configs_from_file();
const char *start_sensores_get_log_file(const char *log_error_path);

void start_sensores_init_can();
int start_sensores_exec_poweron();
int start_sensores_exec_poweroff();
int start_sensores_exec_read_state();

void start_sensores_rm_ip(size_t index);
void start_sensores_edit_ip(size_t index, start_sensors_ip_t *ip);
void start_sensores_edit_can(start_sensors_power_index_t index, start_sensors_can_data_t *can);
const start_sensors_configs_t *start_sensores_get_configs();

int start_sensors_check_ip(int index_ip);

#endif //START_SENSOR_H