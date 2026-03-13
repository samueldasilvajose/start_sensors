#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "start_sensors.h"
#include "start_sensors_types.h"
#include "start_sensors_utils.h"

#define log_error_path START_SENSORS_PATH"/logs/log_error"
#define config_path_default START_SENSORS_PATH"/configs/config_default.yaml"


char *file_config = NULL;


static void
read_parameters(int argc, char **argv)
{
    const char shortsopt[] = "ch";
    const struct option options[] =
    {
		{"config", no_argument, 0, shortsopt[0]},
		{"help", no_argument, 0, shortsopt[1]},

        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    while ((opt = getopt_long_only(argc, argv, shortsopt, options, &option_index)) != -1)
    {
        if (shortsopt[0] == opt)
        {
            size_t nbyte = ((strlen(optarg) + 1) * sizeof(char));
            start_sensors_copy_data((void **) &file_config, (void *) optarg, nbyte);
        }
        else
        {
            printf("\nUsage:  %s  < Optional %c/%s file_path >\n\n", argv[0], options[0].val, options[0].name);
            exit(EXIT_SUCCESS);
        }
    }

    if (!file_config)
    {
        size_t nbyte = (START_SENSORS_STR_LITERAL_LEN(config_path_default) + 1) * sizeof(char);
        start_sensors_copy_data(&file_config, config_path_default, nbyte);
    }
}


int
main(int argc, char **argv)
{
    start_sensors_init_log(log_error_path);

    read_parameters(argc, argv);
    start_sensores_set_fconfigs(file_config);
    start_sensores_extract_configs_from_file();

    ss_set_error_handler((ss_error_handler_t) ss_send_notify_ts);
    start_sensores_init_can();

    if (ss_start(argc, argv))
    {
        ss_fatal_errno("Check os erros ocorridos no arquivo %s.", start_sensores_get_log_file(NULL));
    }
    
    return 0;
}
