#include <stdatomic.h>
#include "ss_ui_error.h"


static atomic_int ss_errno = 0;

SsErrorLevel
ss_get_error()
{
    return atomic_load(&ss_errno);
}

SsErrorLevel
ss_set_error(SsErrorLevel err)
{
    if ((unsigned) err >= SS_ERROR_COUNT)
    {
        return -1;
    }

    return atomic_exchange(&ss_errno, err);
}
