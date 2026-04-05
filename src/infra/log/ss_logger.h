#ifndef SS_LOGGER_H
#define SS_LOGGER_H

#include "../error/ss_error.h"

void ss_init_log();
void ss_salve_log(SsNotifyMsg *noti);
const char *ss_get_log_file(const char *log_path);

#endif // SS_LOGGER_H