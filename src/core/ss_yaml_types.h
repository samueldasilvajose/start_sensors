#ifndef SS_YAML_PARSER_H
#define SS_YAML_PARSER_H

#include <stdlib.h>

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

#endif //SS_YAML_PARSER_H
