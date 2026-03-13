#ifndef START_SENSORS_ERROR_H
#define START_SENSORS_ERROR_H

#include <stdint.h>
#include "gui/ss_error.h"

#define ERRNO_MSG_MAX 512

typedef uint8_t ss_error_mask_t;
typedef void (*ss_error_handler_t)(SsErrorLevel level, const char *message);


void ss_set_error_handler(ss_error_handler_t handler);
void ss_publish_error(SsErrorLevel level, const char *message, ...);
void ss_fatal_errno(char *msg, ...);

//mask errors
#define SS_ERROR_INFO_MASK ((ss_error_mask_t) 1U << SS_ERROR_INFO) //corresponde a SS_ERROR_INFO
#define SS_ERROR_WARNING_MASK ((ss_error_mask_t) 1U << SS_ERROR_WARNING) //corresponde a SS_ERROR_WARNING
#define SS_ERROR_CRITICAL_MASK ((ss_error_mask_t) 1U << SS_ERROR_CRITICAL) //corresponde a SS_ERROR_CRITICAL

static inline void ss_error_clr_err_level(ss_error_mask_t *mask)
{
    if (!mask)
        return;
    *mask = 0x00;
}

static inline void ss_error_active_all_err_level(ss_error_mask_t *mask)
{
    if (!mask)
        return;
    *mask |= (SS_ERROR_INFO_MASK | SS_ERROR_WARNING_MASK | SS_ERROR_CRITICAL_MASK);
}

static inline void ss_error_set_err_level(SsErrorLevel level, ss_error_mask_t *mask)
{
    if (!mask || (unsigned)level >= SS_ERROR_COUNT)
        return;
    *mask |= (ss_error_mask_t)(1U << level);
}

static inline int ss_error_get_err_level(SsErrorLevel level, ss_error_mask_t mask)
{
    if ((unsigned)level >= SS_ERROR_COUNT)
        return -1;

    return ((1U << level & mask) != 0);
}

static inline void ss_error_rm_err_level(SsErrorLevel level, ss_error_mask_t *mask)
{
    if (!mask || (unsigned)level >= SS_ERROR_COUNT)
        return;
    *mask &= ~((ss_error_mask_t)(1U << level));
}

#endif //START_SENSORS_ERROR_H
