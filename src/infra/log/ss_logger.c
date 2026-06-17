#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>

#include "../utils/ss_utils.h"
#include "../../core/ss_core_types.h"

#define dir_logs START_SENSORS_PATH"/logs"
#define log_error_path dir_logs"/log_error"

static bool save_log = false;


const char *
ss_get_log_file(const char *log_path)
{
    static char *file_name = NULL;
    if (!file_name && log_path)
    {
        time_t now = time(NULL);
        struct tm *info = localtime(&now);

        char buf_time[64];
        size_t len_str = strftime(buf_time, sizeof(buf_time), "%d-%m-%Y_%H-%M-%S", info) + strlen(log_path) + 1;
        file_name = (char *) malloc(len_str * sizeof(char));
        ss_test_alloc(file_name);

        snprintf(file_name, len_str, "%s@%s", log_path, buf_time);
    }
    
    return file_name;
}


void
ss_init_log()
{
    if (!ss_dir_exists(dir_logs))
    {
        if (!ss_creat_dir(dir_logs))
        {
            return;
        }
    }
    
    FILE *file_w = fopen(ss_get_log_file(log_error_path), "w");
    if (!file_w)
    {
        ss_publish_error(SS_ERROR_WARNING, "the log file was not created");
        return;
    }

    save_log = true;
    fclose(file_w);
}


void
ss_salve_log(SsNotifyMsg *noti)
{
    static const char *name_log = NULL;
    if (!save_log || (!name_log && !(name_log = ss_get_log_file(NULL))))
    {
        return;
    }
    
    FILE *file_a = fopen(name_log, "a");

    time_t now = time(NULL);
    struct tm *info = localtime(&now);

    char buf_time[64];
    strftime(buf_time, sizeof(buf_time), "%d-%m-%Y_%H-%M-%S", info);
    fprintf(file_a, "[%s] %s: %d %s\n", buf_time, ss_error_to_string(noti->err), noti->err, noti->msg);

    fclose(file_a);
}
