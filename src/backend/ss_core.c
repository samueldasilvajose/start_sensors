#include <yaml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ss_can.h"
#include "ss_core.h"
#include "../core/ss_yaml_types.h"
#include "../infra/utils/ss_utils.h"
#include "../controller/ss_controller.h"


static ss_configs_t global_configs = {0};

static int power_state = 0;
static ss_error_mask_t level_to_notify = SS_ERROR_ERROR_MASK;

static char *fconfigs = NULL;


inline ss_error_mask_t
ss_set_core_level_to_notify(ss_error_mask_t level)
{
	ss_error_mask_t first_level = level_to_notify;
	level_to_notify = level;
	return first_level;
}


void
ss_init_can()
{
    global_configs.power[SS_POWER_ON].sockfd = -1;
    global_configs.power[SS_POWER_OFF].sockfd = -1;
    global_configs.power[SS_READ_STATE].sockfd = -1;
}


int
ss_exec_poweron()
{
    ss_power_t *ptr = &global_configs.power[SS_POWER_ON];
    if (ptr->sockfd < 0)
    {
       ptr->sockfd = ss_connect_can(ptr->can_interface);
       if (ptr->sockfd < 0)
       {
            return -1;
       }
    }
    
    if (!power_state)
    {
        ss_send_frame(ptr->sockfd, &ptr->msg);
        power_state = 1;

        return 0;
    }

    return 1;
}


int
ss_exec_poweroff()
{
    ss_power_t *ptr = &global_configs.power[SS_POWER_OFF];
    if (ptr->sockfd < 0)
    {
        ptr->sockfd = ss_connect_can(ptr->can_interface);
        if (ptr->sockfd < 0)
        {
            return -1;
        }
    }
    
    if (power_state)
    {
        ss_send_frame(ptr->sockfd, &ptr->msg);
        power_state = 0;

        return 0;
    }

    return 1;
}


int
ss_exec_read_state()
{
    ss_power_t *ptr = &global_configs.power[SS_READ_STATE];
    if (ptr->sockfd < 0)
    {
        ptr->sockfd = ss_connect_can(ptr->can_interface);
        if (ptr->sockfd < 0)
        {
            return -1;
        }
    }

    unsigned char mask_poweron = global_configs.power[SS_POWER_ON].msg.data[0];
    int resp = (ss_recv_frame(ptr->sockfd, &ptr->msg) == 0 ? (ptr->msg.data[0] & mask_poweron) : -1);

    return resp;
}


int
ss_check_ip(int index_ip)
{
    char cmd[128];
    char ip_copy[64];

    ss_lock_check_ips_mutex();
    strncpy(ip_copy, global_configs.sensors.ips[index_ip].ip, sizeof(ip_copy));
    ss_unlock_check_ips_mutex();

    snprintf(cmd, SS_AS(cmd), "ping -c 1 -W 3 %s > /dev/null 2>&1", ip_copy);

    int resp = system(cmd);
    if (resp)
    {
        ss_lock_check_ips_mutex();
        if (ss_error_get_err_level(SS_ERROR_ERROR, level_to_notify) > 0)
        {
            ss_publish_error(global_configs.sensors.ips[index_ip].critical_level, "o dispositivo %s não foi encontrado", global_configs.sensors.ips[index_ip].sensor);
        }
        ss_unlock_check_ips_mutex();
    }
    else
    {
        ss_lock_check_ips_mutex();
        if (ss_error_get_err_level(SS_ERROR_ERROR, level_to_notify) > 0)
        {
            ss_publish_error(SS_ERROR_INFO, "o dispositivo %s encontrado", global_configs.sensors.ips[index_ip].sensor);
        }
        ss_unlock_check_ips_mutex();
    }
    
    return (resp == 0);
}


inline const ss_configs_t *
ss_get_configs()
{
    return &global_configs;
}


void
ss_set_fconfigs(const char *file)
{
    if (file && !fconfigs)
    {
        size_t len = strlen(file) + 1;
        ss_copy_data((void **) &fconfigs, file, len * sizeof(char));
        fconfigs[len-1] = '\0'; //confirmação do terminador nulo
    }
}


inline void
ss_rm_ip(size_t index)
{
    if (index == SIZE_MAX || index > IPS_MAX - 1)
    {
        if (ss_error_get_err_level(SS_ERROR_ERROR, level_to_notify) > 0)
        {
            ss_publish_error(SS_ERROR_ERROR, "Index (%ld) fora dos limites, ip não foi removido. Line %d", index, __LINE__);
        }
        return;
    }
    
    ss_lock_check_ips_mutex();
    global_configs.sensors.size--;
    memset(&global_configs.sensors.ips[index], 0, sizeof(ss_sensor_t));
    ss_unlock_check_ips_mutex();
}


inline void
ss_edit_ip(size_t index, ss_sensor_t *ip)
{
    if (index == SIZE_MAX || index > IPS_MAX  - 1 || !ip)
    {
        if (ss_error_get_err_level(SS_ERROR_ERROR, level_to_notify) > 0)
        {
            ss_publish_error(SS_ERROR_ERROR, "Index (%ld) fora dos limites ou ponteiro ss_sensor_t inválido, \
                        ip não foi alterado. Line %d", index, __LINE__);
        }
        return;
    }
    
    ss_lock_check_ips_mutex();
    global_configs.sensors.ips[index] = *ip;
    ss_unlock_check_ips_mutex();
}


inline void
ss_edit_can(ss_power_index_t index, ss_power_t *can)
{
    if ((unsigned) index >= SS_POWER_COUNT || !can)
    {
        if (ss_error_get_err_level(SS_ERROR_ERROR, level_to_notify) > 0)
        {
            ss_publish_error(SS_ERROR_ERROR, "Index (%ld) fora dos limites ou ponteiro ss_power_t inválido, \
                                can não foi alterado. Line %d", index, __LINE__);
        }
        return;
    }
    
    global_configs.power[index] = *can;
}


static void
write_elements_can(FILE *file_w, ss_tag_yaml_t tag, ss_power_t *src)
{
    static char *fmt = "\
\t%s: %d\n\
\t%s: %x\n\
\t%s: %x\n\
\t%s: %s\n";
    char buf[512];
    snprintf(buf, SS_AS(buf), fmt,
             ss_subtag_yaml_to_string(tag, SS_SUBTAG_YAML_CAN_TO_USE), src->to_use,
             ss_subtag_yaml_to_string(tag, SS_SUBTAG_YAML_CAN_FRAME), src->msg.data[0],
             ss_subtag_yaml_to_string(tag, SS_SUBTAG_YAML_CAN_MESSAGE), src->msg.can_id,
             ss_subtag_yaml_to_string(tag, SS_SUBTAG_YAML_CAN_INTERFACE), src->can_interface);
    fprintf(file_w, "%s\n", buf);
}


static void
write_elements_ips(FILE *file_w, ss_sensor_t *src)
{
    static char *fmt = "\
\t\t- %s: %d\n\
\t\t- %s: %s\n\
\t\t- %s: %s\n\
\t\t- %s: %d\n";

    char buf[512];

    ss_lock_check_ips_mutex();
    snprintf(buf, SS_AS(buf), fmt,
             ss_subtag_yaml_to_string(SS_TAG_YAML_SENSORS, SS_SUBTAG_YAML_IP_TO_USE), src->to_use,
             ss_subtag_yaml_to_string(SS_TAG_YAML_SENSORS, SS_SUBTAG_YAML_IP_ID), src->sensor,
             ss_subtag_yaml_to_string(SS_TAG_YAML_SENSORS, SS_SUBTAG_YAML_IP_IP), src->ip,
             ss_subtag_yaml_to_string(SS_TAG_YAML_SENSORS, SS_SUBTAG_YAML_IP_CRITICAL_LEVEL), src->critical_level);
    ss_unlock_check_ips_mutex();
    
    fprintf(file_w, "%s", buf);
}


void
ss_save_fconfigs()
{
    //melhorar a verificação e as funções que escreve o arquivo de configurações
    FILE *file_w = fopen(fconfigs, "w");
    if (!file_w)
    {
        if (ss_error_get_err_level(SS_ERROR_ERROR, level_to_notify) > 0)
        {
            ss_publish_error(SS_ERROR_ERROR, "Não foi possível abrir o arquivo %s, as configurações não foram salvas", fconfigs);
        }
        return;
    }

    fprintf(file_w, "%s:\n", ss_tag_yaml_to_string(SS_TAG_YAML_POWER_ON));
    write_elements_can(file_w, SS_POWER_ON, &global_configs.power[SS_POWER_ON]);

    fprintf(file_w, "%s:\n", ss_tag_yaml_to_string(SS_TAG_YAML_POWER_OFF));
    write_elements_can(file_w, SS_POWER_OFF, &global_configs.power[SS_POWER_OFF]);

    fprintf(file_w, "%s:\n", ss_tag_yaml_to_string(SS_TAG_YAML_READ_STATE));
    write_elements_can(file_w, SS_READ_STATE, &global_configs.power[SS_READ_STATE]);

    fprintf(file_w, "%s:\n", ss_tag_yaml_to_string(SS_TAG_YAML_SENSORS));
    for (short i = 1; i < global_configs.sensors.size; i++)
    {
        fprintf(file_w, "\tsensor%d:\n", i);
        write_elements_ips(file_w, &global_configs.sensors.ips[i-1]);   
    }
    
    fclose(file_w);
}


static void
read_data_can(ss_power_t *dst, ss_tag_yaml_t tag, yaml_event_t *event, yaml_parser_t *parser)
{
    yaml_event_delete(event);

    int ntoken = 0;
    for (ntoken = 0; ntoken < SS_SUBTAG_YAML_CAN_LIST_COUNT;)
    {
        yaml_parser_parse(parser, event);
        if (event->type == YAML_SCALAR_EVENT)
        {
            char *value = (char *) event->data.scalar.value;
            if (strcmp(value, ss_subtag_yaml_to_string(tag, SS_SUBTAG_YAML_CAN_TO_USE)) == 0)
            {
                yaml_event_delete(event), yaml_parser_parse(parser, event);

                int dst_to_use;
                ss_convert_str_to_num(&dst_to_use, (const char *) event->data.scalar.value, INT);
                dst->to_use = dst_to_use ? true : false;
                ntoken++;
            }
            else if (strcmp(value, ss_subtag_yaml_to_string(tag, SS_SUBTAG_YAML_CAN_FRAME)) == 0)
            {
                yaml_event_delete(event), yaml_parser_parse(parser, event);

                dst->msg.can_dlc = 1;
                ss_convert_str_to_num(dst->msg.data, (const char *) event->data.scalar.value, LHEXA);
                ntoken++;
            }
            else if (strcmp(value, ss_subtag_yaml_to_string(tag, SS_SUBTAG_YAML_CAN_MESSAGE)) == 0)
            {
                yaml_event_delete(event), yaml_parser_parse(parser, event);
                ss_convert_str_to_num(&dst->msg.can_id, (const char *) event->data.scalar.value, IHEXA);
                ntoken++;
            }
            else if (strcmp(value, ss_subtag_yaml_to_string(tag, SS_SUBTAG_YAML_CAN_INTERFACE)) == 0)
            {
                yaml_event_delete(event), yaml_parser_parse(parser, event);
                strncpy(dst->can_interface, (const char *) event->data.scalar.value, SS_AS(dst->can_interface));
                ntoken++;
            }
        }

        if (event->type == YAML_MAPPING_END_EVENT)
        {
            break;
        }

        yaml_event_delete(event);
    }

    if (ntoken != SS_SUBTAG_YAML_CAN_LIST_COUNT)
    {
        ss_fatal_errno("Número de tokens lidos diferente do esperado (%d != %d)", ntoken, SS_SUBTAG_YAML_CAN_LIST_COUNT);
    }
}


static size_t
read_data_ips(ss_sensor_t *dst,  yaml_event_t *event, yaml_parser_t *parser)
{
    int ntoken = 0, tag = SS_TAG_YAML_SENSORS;
    size_t index = 0;
    yaml_event_delete(event);
    while (index <= IPS_MAX)
    {
        yaml_parser_parse(parser, event);
        if (event->type == YAML_SCALAR_EVENT)
        {
            char buf[16];
            char *value = (char *) event->data.scalar.value;

            snprintf(buf, SS_AS(buf), "sensor%ld", index+1);

            if (strcmp(value, buf) == 0)
            {
                index++;
                yaml_event_delete(event);
                yaml_parser_parse(parser, event);
            }

            for (ntoken = 0; ntoken < SS_SUBTAG_YAML_IP_LIST_COUNT;)
            {
                yaml_parser_parse(parser, event);
                if (event->type == YAML_SCALAR_EVENT)
                {
                    value = (char *) event->data.scalar.value;
                    if (strcmp(value, ss_subtag_yaml_to_string(tag, SS_SUBTAG_YAML_CAN_TO_USE)) == 0)
                    {
                        yaml_event_delete(event), yaml_parser_parse(parser, event);

                        int dst_to_use;
                        ss_convert_str_to_num(&dst_to_use, (const char *) event->data.scalar.value, INT);
                        dst->to_use = dst_to_use ? true : false;
                        ntoken++;
                    }
                    else if (strcmp(value, ss_subtag_yaml_to_string(tag, SS_SUBTAG_YAML_IP_ID)) == 0)
                    {
                        yaml_event_delete(event), yaml_parser_parse(parser, event);
                        strncpy(dst[index-1].sensor, (const char *) event->data.scalar.value, SS_AS(dst->sensor));
                        ntoken++;
                    }
                    else if (strcmp(value, ss_subtag_yaml_to_string(tag, SS_SUBTAG_YAML_IP_IP)) == 0)
                    {
                        yaml_event_delete(event), yaml_parser_parse(parser, event);
                        strncpy(dst[index-1].ip, (const char *) event->data.scalar.value, SS_AS(dst->ip));
                        ntoken++;
                    }
                    else if (strcmp(value, ss_subtag_yaml_to_string(tag, SS_SUBTAG_YAML_IP_CRITICAL_LEVEL)) == 0)
                    {
                        yaml_event_delete(event), yaml_parser_parse(parser, event);
                        ss_convert_str_to_num(&dst[index-1].critical_level, (const char *) event->data.scalar.value, INT);
                        ntoken++;
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

    if (ntoken != SS_SUBTAG_YAML_IP_LIST_COUNT)
    {
        ss_fatal_errno("Número de tokens lidos diferente do esperado (%d != %d)", ntoken, SS_SUBTAG_YAML_CAN_LIST_COUNT);
    }

    return index;
}


void
ss_extract_configs_from_file()
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

    int ntoken;
    for (ntoken = 0; ntoken < SS_TAG_YAML_LIST_COUNT;)
    {
        yaml_parser_parse(&parser, &event);
        if (event.type == YAML_SCALAR_EVENT)
        {
            char *value = (char *) event.data.scalar.value;
            if (strcmp(value, ss_tag_yaml_to_string(SS_TAG_YAML_POWER_ON)) == 0)
            {
                yaml_event_delete(&event), yaml_parser_parse(&parser, &event);
                read_data_can(&global_configs.power[SS_POWER_ON], SS_POWER_ON, &event, &parser);
                ntoken++;
            }
            else if (strcmp(value, ss_tag_yaml_to_string(SS_TAG_YAML_POWER_OFF)) == 0)
            {
               yaml_event_delete(&event), yaml_parser_parse(&parser, &event);
               read_data_can(&global_configs.power[SS_POWER_OFF], SS_POWER_OFF, &event, &parser);
               ntoken++;
            }
            else if (strcmp(value, ss_tag_yaml_to_string(SS_TAG_YAML_READ_STATE)) == 0)
            {
               yaml_event_delete(&event), yaml_parser_parse(&parser, &event);
               read_data_can(&global_configs.power[SS_READ_STATE], SS_READ_STATE, &event, &parser);
               ntoken++;
            }
            else if (strcmp(value, ss_tag_yaml_to_string(SS_TAG_YAML_SENSORS)) == 0)
            {
               yaml_event_delete(&event), yaml_parser_parse(&parser, &event);
               global_configs.sensors.size = read_data_ips(global_configs.sensors.ips, &event, &parser);
               ntoken++;
            }
        }

        if (event.type == YAML_STREAM_END_EVENT)
        {
            yaml_event_delete(&event);
            break;
        }

        yaml_event_delete(&event);
    }

    if (ntoken != SS_TAG_YAML_LIST_COUNT)
    {
        ss_fatal_errno("Número de tokens lidos diferente do esperado (%d != %d)", ntoken, SS_TAG_YAML_LIST_COUNT);
    }

    yaml_parser_delete(&parser);
    fclose(file_r);
}
