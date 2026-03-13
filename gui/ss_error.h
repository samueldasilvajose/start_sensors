#ifndef SS_ERROR_H
#define SS_ERROR_H

#define SS_ERROR_LIST(X)            \
    X(OK,       "")                  \
    X(INFO,     "INFO")          \
    X(WARNING,  "WARNING")      \
    X(CRITICAL, "CRITICAL")      \
    X(ABORT,    "ABORT")      


typedef enum
{
#define X(name, str) SS_ERROR_##name,
    SS_ERROR_LIST(X)
#undef X
    SS_ERROR_COUNT
} SsErrorLevel;


SsErrorLevel ss_get_error();
SsErrorLevel ss_set_error(SsErrorLevel err);

static inline const char *
ss_error_to_string(SsErrorLevel type)
{
    static const char *_ss_error_strings[] =
{
#define X(name, str) str,
    SS_ERROR_LIST(X)
#undef X
};

    if ((unsigned) type >= SS_ERROR_COUNT)
    {
        return "";
    }
    return _ss_error_strings[type];
}

#endif // SS_ERROR_H