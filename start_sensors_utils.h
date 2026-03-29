#ifndef START_SENSORS_UTILS_H
#define START_SENSORS_UTILS_H


#include "gui/ss_controller.h"
#include "start_sensors_error.h"

#define START_SENSORS_AS(arr) (sizeof(arr) / sizeof((arr)[0]))
#define START_SENSORS_STR_LITERAL_LEN(s) ((sizeof(s)/sizeof(char)) - 1)

#define start_sensors_free(ptr) do { if (*ptr != NULL) { free(*ptr); *ptr = NULL; } } while (0)
#define start_sensors_test_alloc(ptr) do { if (ptr == NULL) { ss_fatal_errno("Memory allocation failed (line: %d)", __LINE__); } } while (0)


typedef enum
{
    INT = 1,
    LONG,
    IHEXA,
    LHEXA,
    DOUBLE
} convert_type_t;


int start_sensors_copy_data(void **dst, const void *src, size_t nbytes);
int start_sensors_convert_str_to_num(void *, const char *, convert_type_t);
size_t extra_keys_read_file(char **buf, const char *file_name);

#endif //START_SENSORS_UTILS_H