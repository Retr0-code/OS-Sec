#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mysyslog.h"

int main(int argc, char **argv) {
    if (mysyslog(
        "Example message",
        loglvl_INFO,
        drv_ascii,
        fmt_txt,
        "/home/nik-korneev/mysyslog"
        ) != slerror_success) {
        fprintf(stderr, "An error occured: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}