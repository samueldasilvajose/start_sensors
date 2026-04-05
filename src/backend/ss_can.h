#ifndef SS_CAN_H
#define SS_CAN_H

#include <linux/can.h>
#include "../infra/error/ss_error_message.h"

void ss_get_frames_string(struct can_frame *frame);

int ss_send_frame(int sockfd, struct can_frame *frame);
int ss_recv_frame(int sockfd, struct can_frame *frame);
int ss_connect_can(char *intf_name);

ss_error_mask_t ss_set_can_level_to_notify(ss_error_mask_t level);

#endif //SS_CAN_H