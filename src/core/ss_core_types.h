#ifndef SART_SENSOR_TYPES_H
#define SART_SENSOR_TYPES_H

#include <stdbool.h>
#include <linux/can.h>

//definido durante compilacao
#ifndef START_SENSORS_PATH
#define START_SENSORS_PATH ""
#endif

//define quantidade de ips maximos que podem ser checados
#define IPS_MAX 16

//define o comprimento maximo de cada tipo de string
#define IP_SIZE 32
#define SENSOR_SIZE 256
#define CAN_INTERFACE_SIZE 32


typedef enum
{
    SS_POWER_ON = 0,
    SS_POWER_OFF,
    SS_READ_STATE,
    SS_POWER_COUNT
} ss_power_index_t;

typedef struct
{
    bool to_use;
    int sockfd;
    char can_interface[CAN_INTERFACE_SIZE];
    struct can_frame msg;
} ss_power_t;

typedef struct
{
    bool to_use;
    char ip[IP_SIZE];
    char sensor[SENSOR_SIZE];
    short critical_level;
} ss_sensor_t;

typedef struct
{
    struct
    {
        short size;
        ss_sensor_t ips[IPS_MAX];
    } sensors;
    
    ss_power_t power[SS_POWER_COUNT];
} ss_configs_t;


#endif // SART_SENSOR_TYPES_H