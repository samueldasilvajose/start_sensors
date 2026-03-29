#ifndef SS_CONTROLLER_H
#define SS_CONTROLLER_H

#include "ss_error.h"
#include "ss_signal_types.h"

#define SS_TAG_YAML_FIELD_COUNT 4

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
} SsTagYaml;

typedef struct
{
    SsErrorLevel err;
    char msg[512];
}SsNotifyMsg;

typedef struct
{
    const char **strings;
    unsigned count;
} SsYamlMap;


// ================== frontend -> backend ================== //

void ss_check_ips();
int ss_poweron_sensors();
int ss_poweroff_sensors();
int ss_read_state_sensors();


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


static inline const char *
ss_tag_yaml_to_string(SsTagYaml type)
{
    static const char *_tag_yaml_strings[] =
{
#define X(name, str) str,
    SS_TAG_YAML_LIST(X)
#undef X
};

    if ((unsigned) type >= SS_TAG_YAML_LIST_COUNT)
    {
        return NULL;
    }
    return _tag_yaml_strings[type];
}

static const char *_common[] = { "to_use", "frame", "message", "interface" };
static const char *_sensor[] = { "to_use", "id", "ip", "critical_level" };

static const SsYamlMap maps[] = \
{
    [SS_TAG_YAML_POWER_ON]   = { _common, SS_TAG_YAML_FIELD_COUNT },
    [SS_TAG_YAML_POWER_OFF]  = { _common, SS_TAG_YAML_FIELD_COUNT },
    [SS_TAG_YAML_READ_STATE] = { _common, SS_TAG_YAML_FIELD_COUNT },
    [SS_TAG_YAML_SENSORS]    = { _sensor, SS_TAG_YAML_FIELD_COUNT },
};

static inline const char *
ss_subtag_yaml_to_string(SsTagYaml type, unsigned subtype)
{
    if ((unsigned)type >= SS_TAG_YAML_LIST_COUNT)
        return NULL;

    const SsYamlMap *map = &maps[type];

    if (!map->strings || subtype >= map->count)
        return NULL;

    return map->strings[subtype];
}

#endif // SS_CONTROLLER_H