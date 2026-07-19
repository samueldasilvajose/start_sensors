#ifndef SS_ERROR_H
#define SS_ERROR_H

#define ERRNO_MSG_MAX 512

#define SS_ERROR_LIST(X)           \
    X(OK,       "")                \
    X(INFO,     "INFO")            \
    X(WARNING,  "WARNING")         \
    X(ERROR,    "ERROR")           \
    X(CRITICAL, "CRITICAL")        \
    X(FATAL,    "FATAL")      


typedef enum
{
#define X(name, str) SS_ERROR_##name,
    SS_ERROR_LIST(X)
#undef X
    SS_ERROR_COUNT
} SsErrorLevel;

static const char *const _ss_error_strings[] =
{
#define X(name, str) str,
    SS_ERROR_LIST(X)
#undef X
};

typedef struct
{
    SsErrorLevel err;
    char msg[ERRNO_MSG_MAX];
}SsNotifyMsg;


static inline const char *
ss_error_to_string(SsErrorLevel type)
{
    if ((unsigned) type >= SS_ERROR_COUNT)
    {
        return "";
    }
    return _ss_error_strings[type];
}

#endif // SS_ERROR_H
