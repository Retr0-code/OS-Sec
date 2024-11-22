#include "mysyslog_back.h"

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "mysyslog_back.h"

char proc_full_name[4096];
size_t proc_name_len = sizeof(proc_full_name);

int mysyslog(const char *msg, int level, int driver, int format, const char *path) {
    if (msg == NULL || path == NULL) {
        errno = EINVAL;
        return slerror_invalid_args;
    }

    const char *lvl_str = NULL;
    const char *fmt_str = NULL;
    for (size_t i = 0; i < levels_amount; ++i) {
        if (level == level_str[i].lvl) {
            lvl_str = level_str[i].str;
            break;
        }
    }

    if (lvl_str == NULL) {
        errno = EINVAL;
        return slerror_invalid_args;
    }

    for (size_t i = 0; i < formats_amount; ++i) {
        if (format == format_str[i].fmt) {
            fmt_str = format_str[i].str;
            break;
        }
    }

    if (fmt_str == NULL) {
        errno = EINVAL;
        return slerror_invalid_args;
    }

    size_t total_len = strlen(path) + strlen(fmt_str);
    char *full_path = malloc(total_len);
    strncpy(full_path, path, total_len);
    strcat(full_path, fmt_str);

    if (proc_full_name[0] == 0)
        get_process_name(proc_full_name, proc_name_len);

    int fd = open(full_path, LOGS_FILE_FLAGS, LOGS_FILE_PERMS);
    free(full_path);
    if (fd == -1)
        return slerror_invalid_args;

    lseek(fd, 0, SEEK_END);
    if (dprintf(fd, "{\"timestamp\": %lu, \"log_level\": \"%s\", \"process\": \"%s\", \"message\": \"%s\"}", time(NULL), lvl_str, proc_full_name, msg) < 0) {
        close(fd);
        return slerror_bad_descriptor;
    }

    close(fd);
    return slerror_success;
}
