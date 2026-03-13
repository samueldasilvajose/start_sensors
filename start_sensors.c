#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include <time.h>
#include <yaml.h>
#include <string.h>

#include "start_sensors.h"
#include "start_sensors_can.h"
#include "start_sensors_error.h"


static start_sensors_configs_t global_configs = {0};

static char *fconfigs = NULL;
static int power_state = 0;

//sudo apt-get install libyaml-dev

void
start_sensores_salve_log(SsNotifyMsg *noti)
{
    static const char *name_log = NULL;
    if (!name_log && !(name_log = start_sensores_get_log_file(NULL)))
    {
        return;
    }
    
    FILE *file_a = fopen(name_log, "a");
    time_t now = time(NULL);
    struct tm *info = localtime(&now);

    char buf_time[64];
    strftime(buf_time, sizeof(buf_time), "%d-%m-%Y_%H-%M-%S", info);
    fprintf(file_a, "%s [%s]: %d %s\n", ss_error_to_string(noti->err), buf_time, noti->err, noti->msg);

    fclose(file_a);
}


const char *
start_sensores_get_log_file(const char *log_error_path)
{
    static char *file_name = NULL;
    if (!file_name && log_error_path)
    {
        time_t now = time(NULL);
        struct tm *info = localtime(&now);

        char buf_time[64];
        size_t len_str = strftime(buf_time, sizeof(buf_time), "%d-%m-%Y_%H-%M-%S", info) + strlen(log_error_path) + 1;
        file_name = (char *) malloc(len_str * sizeof(char));
        start_sensors_test_alloc(file_name);

        snprintf(file_name, len_str, "%s@%s", log_error_path, buf_time);
    }
    
    return file_name;
}


void
start_sensors_init_log(const char *log_error_path)
{
    FILE *file_w = fopen(start_sensores_get_log_file(log_error_path), "w");
    if (!file_w)
    {
        ss_fatal_errno("Log file not created%s", strerror(errno));
    }
    fclose(file_w);
}


void
start_sensores_init_can()
{
    global_configs.power[SS_POWER_ON].sockfd = -1;
    global_configs.power[SS_POWER_OFF].sockfd = -1;
    global_configs.power[SS_READ_STATE].sockfd = -1;
}


void
start_sensores_exec_poweron()
{
    start_sensors_can_data_t *ptr = &global_configs.power[SS_POWER_ON];
    if (ptr->sockfd < 0)
    {
       ptr->sockfd = ss_connect_can(ptr->can_interface);
       if (ptr->sockfd < 0)
       {
            return;
       }
    }
    
    if (!power_state)
    {
        ss_send_frame(ptr->sockfd, &ptr->msg);
        power_state = 1;
    }
}


void
start_sensores_exec_poweroff()
{
    start_sensors_can_data_t *ptr = &global_configs.power[SS_POWER_OFF];
    if (ptr->sockfd < 0)
    {
        ptr->sockfd = ss_connect_can(ptr->can_interface);
        if (ptr->sockfd < 0)
        {
            return;
        }
    }
    
    if (power_state)
    {
        ss_send_frame(ptr->sockfd, &ptr->msg);
        power_state = 0;
    }
}


int
start_sensors_check_ip(int index_ip)
{
    char cmd[128];
    char ip_copy[64];

    ss_lock_check_ips_mutex();
    strncpy(ip_copy, global_configs.ips[index_ip].ip, sizeof(ip_copy));
    ss_unlock_check_ips_mutex();

    snprintf(cmd, START_SENSORS_AS(cmd), "ping -c 1 -W 3 %s > /dev/null 2>&1", ip_copy);

    int resp = system(cmd);
    if (resp)
    {
        ss_lock_check_ips_mutex();
        ss_publish_error(global_configs.ips[index_ip].critical_level, "o dispositivo %s não foi encontrado", global_configs.ips[index_ip].sensor);
        ss_unlock_check_ips_mutex();
    }
    else
    {
        ss_lock_check_ips_mutex();
        ss_publish_error(SS_ERROR_INFO, "o dispositivo %s encontrado", global_configs.ips[index_ip].sensor);
        ss_unlock_check_ips_mutex();
    }
    
    return (resp == 0);
}


int
start_sensores_exec_read_state()
{
    start_sensors_can_data_t *ptr = &global_configs.power[SS_READ_STATE];
    if (ptr->sockfd < 0)
    {
       ptr->sockfd = ss_connect_can(ptr->can_interface);
    }

    unsigned char mask_poweron = global_configs.power[SS_POWER_ON].msg.data[0];
    int resp = (ss_recv_frame(ptr->sockfd, &ptr->msg) == 0 ? (ptr->msg.data[0] & mask_poweron) : -1);

    return resp;
}


inline const start_sensors_configs_t *
start_sensores_get_configs()
{
    return &global_configs;
}


inline void
start_sensores_rm_ip(size_t index)
{
    if (index == SIZE_MAX || index > IPS_MAX - 1)
    {
       return;
    }
    
    ss_lock_check_ips_mutex();
    memset(&global_configs.ips[index], 0, sizeof(start_sensors_ip_t));
    ss_unlock_check_ips_mutex();
}


inline void
start_sensores_edit_ip(size_t index, start_sensors_ip_t *ip)
{
    if (index == SIZE_MAX || index > IPS_MAX  - 1 || !ip)
    {
       return;
    }
    
    ss_lock_check_ips_mutex();
    global_configs.ips[index] = *ip;
    ss_unlock_check_ips_mutex();
}


inline void
start_sensores_edit_can(start_sensors_power_index_t index, start_sensors_can_data_t *can)
{
    if ((unsigned) index > SS_POWER_COUNT || !can)
    {
       return;
    }
    
    global_configs.power[index] = *can;
}


static void
read_data_can(start_sensors_can_data_t *dst,  yaml_event_t *event, yaml_parser_t *parser)
{
    yaml_event_delete(event);
    while (1)
    {
        yaml_parser_parse(parser, event);
        if (event->type == YAML_SCALAR_EVENT)
        {
            if (strcmp(event->data.scalar.value, "frame") == 0)
            {
                yaml_event_delete(event), yaml_parser_parse(parser, event);
                memset(&dst->msg.data, 0, sizeof(char)*8);
                start_sensors_convert_str_to_num(dst->msg.data, event->data.scalar.value, IHEXA);
                dst->msg.can_dlc = 1U;
            }
            else if (strcmp(event->data.scalar.value, "message") == 0)
            {
                yaml_event_delete(event), yaml_parser_parse(parser, event);
                start_sensors_convert_str_to_num(&dst->msg.can_id, event->data.scalar.value, IHEXA);
            }
            else if (strcmp(event->data.scalar.value, "interface") == 0)
            {
                yaml_event_delete(event), yaml_parser_parse(parser, event);
                strncpy(dst->can_interface, event->data.scalar.value, START_SENSORS_AS(dst->can_interface));
            }
        }

        if (event->type == YAML_MAPPING_END_EVENT)
        {
            break;
        }

        yaml_event_delete(event);
    }
}


static size_t
read_data_ips(start_sensors_ip_t *dst,  yaml_event_t *event, yaml_parser_t *parser)
{
    size_t index = 0;
    yaml_event_delete(event);
    while (index <= IPS_MAX)
    {
        yaml_parser_parse(parser, event);
        if (event->type == YAML_SCALAR_EVENT)
        {
            char buf[16];
            snprintf(buf, START_SENSORS_AS(buf), "sensor%ld", index+1);
            if (strcmp(event->data.scalar.value, buf) == 0)
            {
                index++;
                yaml_event_delete(event);
                yaml_parser_parse(parser, event);
            }

            while (1)
            {
                yaml_parser_parse(parser, event);
                if (event->type == YAML_SCALAR_EVENT)
                {
                    if (strcmp(event->data.scalar.value, "id") == 0)
                    {
                        yaml_event_delete(event), yaml_parser_parse(parser, event);
                        strncpy(dst[index-1].sensor, event->data.scalar.value, START_SENSORS_AS(dst->sensor));
                    }
                    else if (strcmp(event->data.scalar.value, "ip") == 0)
                    {
                        yaml_event_delete(event), yaml_parser_parse(parser, event);
                        strncpy(dst[index-1].ip, event->data.scalar.value, START_SENSORS_AS(dst->ip));
                    }
                    else if (strcmp(event->data.scalar.value, "critical_level") == 0)
                    {
                        yaml_event_delete(event), yaml_parser_parse(parser, event);
                        start_sensors_convert_str_to_num(&dst[index-1].critical_level, event->data.scalar.value, INT);
                    }
                }

                if (event->type == YAML_SEQUENCE_END_EVENT)
                {
                    break;
                }

                yaml_event_delete(event);
            }
        }

        if (event->type == YAML_MAPPING_END_EVENT)
        {
            break;
        }

        yaml_event_delete(event);
    }
    return index;
}


void
start_sensores_set_fconfigs(const char *file)
{
    if (file && !fconfigs)
    {
        size_t len = strlen(file) + 1;
        start_sensors_copy_data(&fconfigs, file, len * sizeof(char));
        fconfigs[len-1] = '\0'; //confirmação do terminador nulo
    }
}


static void
write_elements_can(FILE *file_w, start_sensors_can_data_t *src)
{
    static char *fmt = "\
\tframe: %x\n\
\tmessage: %x\n\
\tinterface: %s\n";
    char buf[512];
    snprintf(buf, START_SENSORS_AS(buf), fmt, src->msg.data[0], src->msg.can_id, src->can_interface);
    fprintf(file_w, "%s\n", buf);
}


static void
write_elements_ips(FILE *file_w, start_sensors_ip_t *src)
{
    static char *fmt = "\
\t\t- id: %s\n\
\t\t- ip: %s\n\
\t\t- critical_leve: %d\n";

    char buf[512];

    ss_lock_check_ips_mutex();
    snprintf(buf, START_SENSORS_AS(buf), fmt, src->sensor, src->ip, src->critical_level);
    ss_unlock_check_ips_mutex();
    
    fprintf(file_w, "%s", buf);
}


void
start_sensores_save_fconfigs()
{
    FILE *file_w = fopen(fconfigs, "w");
    if (!file_w)
    {
        return;
    }

    fprintf(file_w, "%s:\n", ss_tag_yaml_to_string(SS_TAG_YAML_POWER_ON));
    write_elements_can(file_w, &global_configs.power[SS_POWER_ON]);

    fprintf(file_w, "%s:\n", ss_tag_yaml_to_string(SS_TAG_YAML_POWER_OFF));
    write_elements_can(file_w, &global_configs.power[SS_POWER_OFF]);

    fprintf(file_w, "%s:\n", ss_tag_yaml_to_string(SS_TAG_YAML_READ_STATE));
    write_elements_can(file_w, &global_configs.power[SS_READ_STATE]);

    fprintf(file_w, "%s:\n", ss_tag_yaml_to_string(SS_TAG_YAML_SENSORS));
    for (size_t i = 1; i < global_configs.ips_size; i++)
    {
        fprintf(file_w, "\tsensor%ld:\n", i);
        write_elements_ips(file_w, &global_configs.ips[i-1]);   
    }
    
    fclose(file_w);
}


void
start_sensores_extract_configs_from_file()
{
    if (!fconfigs)
    {
        ss_fatal_errno("Arquivo de configuração não encontrado");
    }
    
    FILE *file_r = fopen(fconfigs, "r");
    yaml_event_t event;
    yaml_parser_t parser;

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, file_r);

    while (1)
    {
        yaml_parser_parse(&parser, &event);
        if (event.type == YAML_SCALAR_EVENT)
        {
            if (strcmp(event.data.scalar.value, ss_tag_yaml_to_string(SS_TAG_YAML_POWER_ON)) == 0)
            {
                yaml_event_delete(&event), yaml_parser_parse(&parser, &event);
                read_data_can(&global_configs.power[SS_POWER_ON], &event, &parser);
            }
            else if (strcmp(event.data.scalar.value, ss_tag_yaml_to_string(SS_TAG_YAML_POWER_OFF)) == 0)
            {
               yaml_event_delete(&event), yaml_parser_parse(&parser, &event);
               read_data_can(&global_configs.power[SS_POWER_OFF], &event, &parser);
            }
            else if (strcmp(event.data.scalar.value, ss_tag_yaml_to_string(SS_TAG_YAML_READ_STATE)) == 0)
            {
               yaml_event_delete(&event), yaml_parser_parse(&parser, &event);
               read_data_can(&global_configs.power[SS_READ_STATE], &event, &parser);
            }
            else if (strcmp(event.data.scalar.value, ss_tag_yaml_to_string(SS_TAG_YAML_SENSORS)) == 0)
            {
               yaml_event_delete(&event), yaml_parser_parse(&parser, &event);
               global_configs.ips_size = read_data_ips(global_configs.ips, &event, &parser);
            }
        }

        if (event.type == YAML_STREAM_END_EVENT)
        {
            yaml_event_delete(&event);
            break;
        }

        yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);
    fclose(file_r);
}
