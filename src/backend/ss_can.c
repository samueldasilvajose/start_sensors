#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <fcntl.h>
#include <unistd.h>

#ifndef __USE_MISC
#define __USE_MISC
#include <net/if.h>
#undef __USE_MISC
#else
#include <net/if.h>
#endif

#include <linux/can/raw.h>
#include "ss_can.h"


static ss_error_mask_t level_to_notify = SS_ERROR_CRITICAL_MASK;


inline ss_error_mask_t
ss_set_can_level_to_notify(ss_error_mask_t level)
{
	ss_error_mask_t first_level = level_to_notify;
	level_to_notify = level;
	return first_level;
}


void
ss_get_frames_string(struct can_frame *frame)
{
	size_t current_pos = 0;
	size_t frame_show_size = 512;

	char frame_show[frame_show_size];
	current_pos += snprintf(frame_show + current_pos, frame_show_size - current_pos, "%04x: ", frame->can_id);

	if (frame->can_id & CAN_RTR_FLAG)
	{
		current_pos += snprintf(frame_show + current_pos, frame_show_size - current_pos, "remote request");
	} else
	{
		current_pos += snprintf(frame_show + current_pos, frame_show_size - current_pos, "[%d]", frame->can_dlc);

		for (int i = 0; i < frame->can_dlc; i++)
		{
			current_pos += snprintf(frame_show + current_pos, frame_show_size - current_pos, " %02x", frame->data[i]);
		}
	}

	ss_publish_error(SS_ERROR_INFO, frame_show);
}


int
ss_recv_frame(int sockfd, struct can_frame *frame)
{
	errno = 0;
	int ret = recv(sockfd, frame, sizeof(*frame), 0);

	if (ret != sizeof(*frame))
	{
		if (ret < 0)
		{
			if (ss_error_get_err_level(SS_ERROR_WARNING, level_to_notify) > 0)
				ss_publish_error(SS_ERROR_WARNING, "Recv failed (%s)", strerror(errno));
		}
		else
		{
			if (ss_error_get_err_level(SS_ERROR_WARNING, level_to_notify) > 0)
				ss_publish_error(SS_ERROR_WARNING, "Recv returned %d (%s)", ret, strerror(errno));
		}

		return 1;
	}

	return 0;
}


int
ss_recv_last_frame(int sockfd, struct can_frame *frame)
{
    int received = 1;

	while (1)
    {
        int nbytes = recv(sockfd, frame, sizeof(*frame), 0);
        if (nbytes > 0)
        {
            received = 0;
            continue;
		}

		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			ss_publish_error(SS_ERROR_WARNING, "Recv failed (%s)", strerror(errno));
			return 1;
		}

		break;
    }

	return received;
}


int
ss_recv_frame_filter_msg(int sockfd, canid_t can_id, struct can_frame *frame)
{
    int received = 1;
	struct can_frame current_frame = {0};

	while (1)
    {
        int nbytes = recv(sockfd, &current_frame, sizeof(current_frame), 0);
        if (nbytes > 0)
        {
			if (current_frame.can_id == can_id)
			{
				*frame = current_frame;
				received = 0;
			}
			
            continue;
		}

		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			ss_publish_error(SS_ERROR_WARNING, "Recv failed (%s)", strerror(errno));
			return 1;
		}

		break;
    }

	return received;
}


int
ss_send_frame(int sockfd, struct can_frame *frame)
{
	ssize_t ret;
	ssize_t frame_size = (ssize_t) sizeof(*frame);

	errno = 0;
	while ((ret = send(sockfd, frame, sizeof(*frame), 0)) != frame_size)
	{
		if (ret < 0)
		{
			if (errno != ENOBUFS)
			{
				if (ss_error_get_err_level(SS_ERROR_WARNING, level_to_notify) > 0)
				{
					ss_publish_error(SS_ERROR_WARNING, "Send failed (%s)", strerror(errno));
				}
				
				return -1;
			}
		} else
		{
			if (ss_error_get_err_level(SS_ERROR_WARNING, level_to_notify) > 0)
			{
				ss_publish_error(SS_ERROR_WARNING, "Send returned %d", ret);
			}

			return 1;
		}
	}

	return 0;
}


int 
ss_connect_can(char *intf_name)
{
	static int family = PF_CAN, type = SOCK_RAW, proto = CAN_RAW;

	struct ifreq ifr;
	struct sockaddr_can addr;

	if (ss_error_get_err_level(SS_ERROR_INFO, level_to_notify) > 0)
	{
		ss_publish_error(SS_ERROR_INFO, "Interface (%s): family = %d, type = %d, proto = %d",
							intf_name, family, type, proto);
	}

	int sockfd = socket(family, type, proto);
	if (sockfd < 0) 
	{
		if (ss_error_get_err_level(SS_ERROR_CRITICAL, level_to_notify) > 0)
			ss_publish_error(SS_ERROR_CRITICAL, "Not connected %s (%s)", intf_name, strerror(errno));
		return -1;
	}

	strcpy(ifr.ifr_name, intf_name);
	ioctl(sockfd, SIOCGIFINDEX, &ifr);

	int flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	addr.can_family = family;
	addr.can_ifindex = ifr.ifr_ifindex;

	int err = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
	if (err < 0)
	{
		if (ss_error_get_err_level(SS_ERROR_CRITICAL, level_to_notify) > 0)
			ss_publish_error(SS_ERROR_CRITICAL, "Not connected %s (%s)", intf_name, strerror(errno));
		return -1;
	}
	
	if (ss_error_get_err_level(SS_ERROR_INFO, level_to_notify) > 0)
	{
		ss_publish_error(SS_ERROR_INFO, "Successfully connected to %s", intf_name);
	}

	return sockfd;
}
