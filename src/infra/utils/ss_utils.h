#ifndef START_SENSORS_UTILS_H
#define START_SENSORS_UTILS_H

#include <stdbool.h>
#include "../error/ss_error_message.h"

#define SS_AS(arr) (sizeof(arr) / sizeof((arr)[0]))
#define SS_STR_LITERAL_LEN(s) ((sizeof(s)/sizeof(char)) - 1)

#define ss_free(ptr) do { if (*ptr != NULL) { free(*ptr); *ptr = NULL; } } while (0)
#define ss_test_alloc(ptr) do { if (ptr == NULL) { ss_fatal_errno("Memory allocation failed (line: %d)", __LINE__); } } while (0)


typedef enum
{
    INT = 1,
    LONG,
    IHEXA,
    LHEXA,
    DOUBLE
} ss_convert_type_t;


bool ss_creat_dir(const char *path);
bool ss_dir_exists(const char *path);

short ss_copy_data(void **dst, const void *src, size_t nbytes);
short ss_convert_str_to_num(void *, const char *, ss_convert_type_t);

size_t ss_read_file(char **buf, const char *file_name);

#endif //START_SENSORS_UTILS_H
