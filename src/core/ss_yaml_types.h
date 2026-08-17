#ifndef SS_YAML_PARSER_H
#define SS_YAML_PARSER_H

#include <stdlib.h>

#ifdef USE_YAML_SUBTAG_SETTERS
#include "../infra/utils/ss_utils.h"
#endif


//tasg principais do yaml
#define SS_TAG_YAML_LIST(X) \
    X(POWER_ON,         "poweron") \
    X(POWER_OFF,        "poweroff") \
    X(READ_STATE,       "read_status") \
    X(SENSORS,          "sensors")         

typedef enum
{
#define X(name, str) SS_TAG_YAML_##name,
    SS_TAG_YAML_LIST(X)
#undef X
    SS_TAG_YAML_LIST_COUNT
} ss_tag_yaml_t;

static const char * const _tag_yaml_strings[] =
{
#define X(name, str) str,
    SS_TAG_YAML_LIST(X)
#undef X
};


//subtags do yaml (campos can)
#define SS_SUBTAG_YAML_CAN_LIST(X) \
    X(TO_USE,         "to_use") \
    X(FRAME,          "frame") \
    X(MESSAGE,        "message") \
    X(INTERFACE,      "interface")         

typedef enum
{
#define X(name, str) SS_SUBTAG_YAML_CAN_##name,
    SS_SUBTAG_YAML_CAN_LIST(X)
#undef X
    SS_SUBTAG_YAML_CAN_LIST_COUNT
} ss_subtag_yaml_can_t;

static const char * const _can_fields[] =
{
#define X(name, str) str,
    SS_SUBTAG_YAML_CAN_LIST(X)
#undef X
};

//subtags do yaml (campos ip)
#define SS_SUBTAG_YAML_IP_LIST(X) \
    X(TO_USE,           "to_use") \
    X(ID,               "id") \
    X(IP,               "ip") \
    X(CRITICAL_LEVEL,   "critical_level")         

typedef enum
{
#define X(name, str) SS_SUBTAG_YAML_IP_##name,
    SS_SUBTAG_YAML_IP_LIST(X)
#undef X
    SS_SUBTAG_YAML_IP_LIST_COUNT
} ss_subtag_yaml_ip_t;

static const char * const _ip_fields[] =
{
#define X(name, str) str,
    SS_SUBTAG_YAML_IP_LIST(X)
#undef X
};


typedef struct
{
    const char * const *strings;
    unsigned count;
} ss_yaml_map_t;


static inline const char *
ss_tag_yaml_to_string(ss_tag_yaml_t type)
{
    if ((unsigned) type >= SS_TAG_YAML_LIST_COUNT)
    {
        return NULL;
    }

    return _tag_yaml_strings[type];
}

static const ss_yaml_map_t maps[] = \
{
    [SS_TAG_YAML_POWER_ON]   = { _can_fields, SS_SUBTAG_YAML_CAN_LIST_COUNT },
    [SS_TAG_YAML_POWER_OFF]  = { _can_fields, SS_SUBTAG_YAML_CAN_LIST_COUNT },
    [SS_TAG_YAML_READ_STATE] = { _can_fields, SS_SUBTAG_YAML_CAN_LIST_COUNT },
    [SS_TAG_YAML_SENSORS]    = { _ip_fields, SS_SUBTAG_YAML_IP_LIST_COUNT },
};


static inline int
ss_get_qtd_subtags_yaml(ss_tag_yaml_t type)
{
    if ((unsigned) type >= SS_TAG_YAML_LIST_COUNT)
    {
        return -1;
    }
    
    return maps[type].count;
}


static inline const char *
ss_subtag_yaml_to_string(ss_tag_yaml_t type, size_t subtype)
{
    if ((unsigned)type >= SS_TAG_YAML_LIST_COUNT)
        return NULL;

    const ss_yaml_map_t *map = &maps[type];

    if (subtype >= map->count)
        return NULL;

    return map->strings[subtype];
}


static inline void *
ss_get_ptr_subtag_yaml_ip(ss_sensor_t *config, size_t subtype)
{
    void *data = NULL;
    if (!config || subtype >= SS_SUBTAG_YAML_IP_LIST_COUNT)
        return data;

    switch (subtype)
    {
        case SS_SUBTAG_YAML_IP_TO_USE:
            data = &config->to_use; break;
        case SS_SUBTAG_YAML_IP_ID:
            data = &config->sensor; break;
        case SS_SUBTAG_YAML_IP_IP:
            data = &config->ip; break;
        case SS_SUBTAG_YAML_IP_CRITICAL_LEVEL:
            data = &config->critical_level; break;
        default:
            break;
    }

    return data;
}

static inline bool
ss_get_str_subtag_yaml_ip(ss_sensor_t *config, size_t subtype, char *buf, size_t size)
{
    if (!config || !buf || size == 0 || subtype >= SS_SUBTAG_YAML_IP_LIST_COUNT)
        return false;

    switch (subtype)
    {
        case SS_SUBTAG_YAML_IP_TO_USE:
            snprintf(buf, size, "%d", config->to_use); break;
        case SS_SUBTAG_YAML_IP_ID:
            snprintf(buf, size, "%s", config->sensor); break;
        case SS_SUBTAG_YAML_IP_IP:
            snprintf(buf, size, "%s", config->ip); break;
        case SS_SUBTAG_YAML_IP_CRITICAL_LEVEL:
            snprintf(buf, size, "%d", config->critical_level); break;
        default:
            return false;
    }

    return true;
}

#ifdef USE_YAML_SUBTAG_SETTERS
static inline bool
ss_set_str_subtag_yaml_ip(ss_sensor_t *config, size_t subtype, const char *buf)
{
    if (!config || !buf || subtype >= SS_SUBTAG_YAML_IP_LIST_COUNT)
        return false;

    switch (subtype)
    {
        case SS_SUBTAG_YAML_IP_TO_USE:
        {
            int active;
            ss_convert_str_to_num(&active, buf, INT);
            config->to_use = active;
            break;
        }
        case SS_SUBTAG_YAML_IP_ID:
        {
            snprintf(config->sensor, sizeof(config->sensor), "%s", buf);
            break;
        }
        case SS_SUBTAG_YAML_IP_IP:
        {
            snprintf(config->ip, sizeof(config->ip), "%s", buf);
            break;
        }
        case SS_SUBTAG_YAML_IP_CRITICAL_LEVEL:
        {
            int critical_level;
            ss_convert_str_to_num(&critical_level, buf, INT);
            config->critical_level = critical_level;
            break;
        }
        default:
            return false;
    }

    return true;
}
#endif


static inline void *
ss_get_ptr_subtag_yaml_can(ss_power_t *config, size_t subtype)
{
    void *data = NULL;
    if (!config || subtype >= SS_SUBTAG_YAML_CAN_LIST_COUNT)
        return data;

    switch (subtype)
    {
        case SS_SUBTAG_YAML_CAN_TO_USE:
            data = &config->to_use; break;
        case SS_SUBTAG_YAML_CAN_FRAME:
            data = &config->msg.data[0]; break;
        case SS_SUBTAG_YAML_CAN_MESSAGE:
            data = &config->msg.can_id; break;
        case SS_SUBTAG_YAML_CAN_INTERFACE:
            data = &config->can_interface; break;
        default:
            break;
    }

    return data;
}

static inline bool
ss_get_str_subtag_yaml_can(ss_power_t *config, size_t subtype, char *buf, size_t size)
{
    if (!config || !buf || size == 0 || subtype >= SS_SUBTAG_YAML_CAN_LIST_COUNT)
        return false;

    switch (subtype)
    {
        case SS_SUBTAG_YAML_CAN_TO_USE:
            snprintf(buf, size, "%d", config->to_use); break;
        case SS_SUBTAG_YAML_CAN_FRAME:
            snprintf(buf, size, "%x", config->msg.data[0]); break;
        case SS_SUBTAG_YAML_CAN_MESSAGE:
            snprintf(buf, size, "%x", config->msg.can_id); break;
        case SS_SUBTAG_YAML_CAN_INTERFACE:
            snprintf(buf, size, "%s", config->can_interface); break;
        default:
            return false;
    }

    return true;
}

#ifdef USE_YAML_SUBTAG_SETTERS
static inline bool
ss_set_str_subtag_yaml_can(ss_power_t *config, size_t subtype, const char *buf)
{
    if (!config || !buf || subtype >= SS_SUBTAG_YAML_CAN_LIST_COUNT)
        return false;

    switch (subtype)
    {
        case SS_SUBTAG_YAML_CAN_TO_USE:
        {
            int active;
            ss_convert_str_to_num(&active, buf, INT);
            config->to_use = active;
            break;
        }
        case SS_SUBTAG_YAML_CAN_FRAME:
        {
            ss_convert_str_to_num(config->msg.data, buf, LHEXA);
            break;
        }
        case SS_SUBTAG_YAML_CAN_MESSAGE:
        {
            ss_convert_str_to_num(&config->msg.can_id, buf, IHEXA);
            break;
        }
        case SS_SUBTAG_YAML_CAN_INTERFACE:
        {
            snprintf(config->can_interface, sizeof(config->can_interface), "%s", buf);
            break;
        }
        default:
            return false;
    }

    return true;
}
#endif


#endif //SS_YAML_PARSER_H
