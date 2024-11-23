/** @file mysyslog_daemon.c
 *  @brief daemon executable for libmysyslog library
 */
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include <mysyslog.h>
#include <mysyslog_back.h>

#define PID_FILE    "/run/mysyslog.pid"
#define CONFIG_FILE "/etc/mysyslog/mysyslog.conf"

#ifndef SIGINFO
#define SIGINFO SIGPWR
#endif

typedef struct {
    e_driver    driver;
    e_format    format;
    char        *path;
} settings;

static settings daemon_settings = { -1, -1, NULL };

static const char *messages[] = {
    "Message1",
    "Message2",
    "Message3",
    "Message4",
    "Message5"
};
size_t messages_amount = sizeof(messages) / sizeof(*messages);

void signal_handler_terminate(int signal);

int read_config(void);

int main(int argc, char **argv) {
    signal(SIGTERM, signal_handler_terminate);
    signal(SIGINT,  signal_handler_terminate);
    signal(SIGKILL, signal_handler_terminate);

    if (read_config() != 0)
        return ENOENT;

    printf("Starting using driver=%i, format=%i, path=%s\n",
        daemon_settings.driver, daemon_settings.format, daemon_settings.path);

    time_t start = time(NULL), now = 0;
    while (1) {
        if (start - now >= 30) {
            start = now;
            srand(now);
            int num = rand();
            int level = num % levels_amount;
            const char *msg = messages[num % messages_amount];
            
            int status = 0;
            if ((status = mysyslog(msg, level, 
                daemon_settings.driver,
                daemon_settings.format,
                daemon_settings.path)) != slerror_success) {
                return status;
            }
        }
        now = time(NULL);
    }

    free(daemon_settings.path);
    return 0;
}

void signal_handler_terminate(int signal) {
    if (daemon_settings.path)
        free(daemon_settings.path);
    
    switch (signal) {
        case SIGTERM:
            exit(0);
        case SIGKILL:
            exit(-1);
        case SIGINT:
            exit(1);
        default:
            exit(-1);
    }
}

int read_config(void) {
    FILE *config = fopen(CONFIG_FILE, "r");
    if (config == NULL)
        return 1;

    struct option parameters[] = {
        { "driver", required_argument, NULL, 'd' },
        { "format", required_argument, NULL, 'f' },
        { "path",   required_argument, NULL, 'p' },
        { 0, 0, 0, 0 }
    };
    int argc = 1;
    const int required_argc = sizeof(parameters) / sizeof(*parameters);
    char *argv[required_argc];
    argv[0] = "/";
    
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, config) != -1) {
        size_t parameter_len = strlen(line) + 2;
        argv[argc] = malloc(parameter_len);
        argv[argc][0] = '-';
        argv[argc][1] = '-';
        strncpy(argv[argc] + 2, line, parameter_len);
        argv[argc][parameter_len - 1] = 0;
        
        if (line)
            free(line);

        ++argc;
    }
    fclose(config);
    if (argc < required_argc) {
        for (; argc >= 0; --argc)
            free(argv[argc]);

        return 2;
    }

    int arg_index = 0;
    int option = 0;
    while((option = getopt_long(argc, argv, "", parameters, &arg_index)) != -1) {
        switch (option) {
            case 'd':
                daemon_settings.driver = enum_from_str(optarg, driver_names, driver_names_len);
                if (daemon_settings.driver == -1)
                    return 3;

                break;
            
            case 'f':
                daemon_settings.format = enum_from_str(optarg, format_str, formats_amount);
                if (daemon_settings.format == -1)
                    return 4;

                break;

            case 'p':
                if (optarg == NULL)
                    return 5;
                
                size_t path_len = strlen(optarg) + 1;
                daemon_settings.path = malloc(path_len);
                strncpy(daemon_settings.path, optarg, path_len);
                break;

            default:
                return 6;
        }
    }

    for (; argc > 1; --argc)
        free(argv[argc - 1]);
    
    return 0;
}

