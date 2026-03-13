#ifndef START_SENSORS_CAN_H
#define START_SENSORS_CAN_H

#include "start_sensors_types.h"
#include "start_sensors_error.h"

void ss_get_frames_string(struct can_frame *frame);

int ss_send_frame(int sockfd, struct can_frame *frame);
int ss_recv_frame(int sockfd, struct can_frame *frame);
int ss_connect_can(char *intf_name);

ss_error_mask_t ss_set_level_to_notify(ss_error_mask_t level);

#endif //START_SENSORS_CAN_H