#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "ss_error_message.h"


struct error_dispatcher
{
    ss_error_handler_t handler;
};

static struct error_dispatcher dispatcher = {0};


void
ss_set_error_handler(ss_error_handler_t handler)
{
    dispatcher.handler = handler;
}


void
ss_publish_error(SsErrorLevel level, const char *message, ...)
{
    static const int vmsg_size = 512;
    char vmsg[vmsg_size];

    va_list args;
    va_start(args, message);
    vsnprintf(vmsg, vmsg_size - 1, message, args);
    va_end(args);

    if (dispatcher.handler)
    {
        dispatcher.handler(level, vmsg);
    }
    else
    {
        fprintf(stderr, "%s\n", vmsg);
    }
}


void
ss_fatal_errno(char *msg, ...)
{
    int len_str = ERRNO_MSG_MAX + 64;

    char cmd[len_str];
    char buf[ERRNO_MSG_MAX];

    va_list args;
    va_start(args, msg);
    vsnprintf(buf, ERRNO_MSG_MAX, msg, args);
    va_end(args);

    snprintf(cmd, len_str, "zenity --error --text=\"Error: %s\" --ellipsize", buf);
    system(cmd);

    exit(EXIT_FAILURE);
}
