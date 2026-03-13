#ifndef SART_SENSOR_TYPES_H
#define SART_SENSOR_TYPES_H

#include <linux/can.h>

#ifndef START_SENSORS_PATH
#define START_SENSORS_PATH ""
#endif

#define IPS_MAX 16


typedef struct
{
    int sockfd;
    char can_interface[32];
    struct can_frame msg;
} start_sensors_can_data_t;

typedef struct
{
    char ip[32];
    char sensor[256];
    int critical_level;
} start_sensors_ip_t;

typedef enum
{
    SS_POWER_ON = 0,
    SS_POWER_OFF,
    SS_READ_STATE,
    SS_POWER_COUNT
} start_sensors_power_index_t;

typedef struct
{
    size_t ips_size;
    start_sensors_ip_t ips[IPS_MAX];
    start_sensors_can_data_t power[SS_POWER_COUNT];
}start_sensors_configs_t;


#endif // SART_SENSOR_TYPES_H