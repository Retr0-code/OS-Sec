#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "mysyslog.h"

typedef struct {
    int         enum_val;
    const char *str;
} enum_str_pair;

enum_str_pair driver_so_filename_str[] = {
    { drv_ascii,    "libmysyslog_text.so" },
    { drv_json,     "libmysyslog_json.so" },
};
unsigned long int driver_so_filename_amount = sizeof(driver_so_filename_str) / sizeof(enum_str_pair);

enum_str_pair level_str[] = {
    { loglvl_DEBUG,     "DEBUG" },
    { loglvl_INFO,      "INFO" },
    { loglvl_WARN,      "WARN" },
    { loglvl_ERROR,     "ERROR" },
    { loglvl_CRITICAL,  "CRITICAL" }
};
unsigned long int levels_amount = sizeof(level_str) / sizeof(enum_str_pair);

enum_str_pair format_str[] = {
    { fmt_none, "" },
    { fmt_txt,  ".txt" },
    { fmt_log,  ".log" },
    { fmt_json, ".json" },
};
unsigned long int formats_amount = sizeof(format_str) / sizeof(enum_str_pair);

enum_str_pair driver_names[] = {
    { drv_ascii,    "ascii" },
    { drv_json,     "json" }
};
size_t driver_names_len = sizeof(driver_names) / sizeof(enum_str_pair);

void show_usage(void);

void show_options_args(const char *msg, const enum_str_pair *array, size_t options_len);

int enum_from_str(const char *str, const enum_str_pair *array, size_t array_size);

int main(int argc, char **argv) {
    int driver = drv_ascii;
    int loglvl = loglvl_INFO;
    int outfmt = fmt_log;
    const char *msg = NULL;
    const char *path = NULL;

    struct option arguments[] = {
        { "driver", optional_argument, NULL, 'd' },
        { "level",  optional_argument, NULL, 'l' },
        { "format", optional_argument, NULL, 'f' },
        { "msg",    required_argument, NULL, 'm' },
        { "path",   required_argument, NULL, 'p' },
        { 0, 0, 0, 0 }
    };

    int arg_index = 0;
    int option = 0;
    while((option = getopt_long(argc, argv, "", arguments, &arg_index)) != -1) {
        switch (option) {
            case 'd':
                driver = enum_from_str(optarg, driver_names, driver_names_len);
                if (driver == -1) {
                    show_options_args("Drivers available:", driver_names, driver_names_len);
                    return -1;
                }
                break;

            case 'l':
                loglvl = enum_from_str(optarg, level_str, levels_amount);
                if (loglvl == -1) {
                    show_options_args("Log-levels available:", level_str, levels_amount);
                    return -1;
                }
                break;
            
            case 'f':
                outfmt = enum_from_str(optarg, format_str, formats_amount);
                if (outfmt == -1) {
                    show_options_args("File formats available:", format_str, formats_amount);
                    return -1;
                }
                break;

            case 'm':
                msg = optarg;
                break;

            case 'p':
                path = optarg;
                break;

            default:
                show_usage();
                return -1;
        }
    }

    if (msg == NULL) {
        puts("Message is not supplied");
        show_usage();
        return -1;
    }

    if (path == NULL) {
        puts("Output path is not supplied");
        show_usage();
        return -1;
    }

    if (mysyslog(msg, loglvl, driver, outfmt, path) != slerror_success) {
        fprintf(stderr, "An error occured: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}


void show_usage(void) {
    puts("Usage:");
    puts("--driver\toptional\tselects output driver (default ascii)");
    puts("--level\t\toptional\tselects logging level (default INFO)");
    puts("--format\toptional\tselects output file format (default log)");
    puts("--msg\t\trequired\tsets output message");
    puts("--path\t\trequired\tsets output file path");
}

void show_options_args(const char *msg, const enum_str_pair *array, size_t options_len) {
    puts(msg);
    for (size_t i = 0; i < options_len; ++i)
        puts(array[i].str);
}

int enum_from_str(const char *str, const enum_str_pair *array, size_t array_size) {
    if (str == NULL)
        return -1;
    
    int enum_val = -1;
    for (size_t i = 0; i < array_size; ++i) {
        if (strcmp(array[i].str, str) == 0) {
            enum_val = array[i].enum_val;
            break;
        }
    }
    return enum_val;
}
