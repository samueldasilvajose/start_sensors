#ifndef SS_UI_ERROR_H
#define SS_UI_ERROR_H

#include "../infra/error/ss_error.h"


SsErrorLevel ss_get_error();
SsErrorLevel ss_set_error(SsErrorLevel err);

#endif // SS_UI_ERROR_H
