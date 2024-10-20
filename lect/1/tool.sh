#!/bin/bash
long_args_str="help,users,processes,log:,errors:"
short_args_str="hupl:e:"

help_flag=0
users=0
processes=0
out_path=""
err_path=""

cat="/usr/bin/cat"
sort="/usr/bin/sort"
sed="/usr/bin/sed"
cut="/usr/bin/cut"
awk="/usr/bin/awk"

# Shows help menu
show_help() {
    echo -e "$0 [PARAMETERS]"
    echo -e "--help\t\t-h\t\tshows this help message"
    echo -e "--users\t\t-u\t\tshows users list and their home directories"
    echo -e "--processes\t-p\t\tlist process sorted by PID"
    echo -e "--log [PATH]\t-l [PATH]\tredirects stdout to specified file"
    echo -e "--errors [PATH]\t-e [PATH]\tredirects stderr to specified file"
}

parse_arguments() {
    # Check if arguments were passed
    if [ $# -eq 0 ]; then
        echo "No arguments were passed"
        show_help
        exit 1
    fi

    # Parsing options using getopt and globaly defined variables with arguments names
    local options=$(getopt -o $short_args_str -l $long_args_str -- "$@")
    # If error occured while parsing arguments (getopt returned not 0)
    if [ $? -ne 0 ]; then
        echo "Error while parsing arguments"
        show_help
        exit 1
    fi
    eval set -- "$options"

    # Read the named argument values
    while [ $# -gt 0 ]; do
        case "$1" in
            "-h"|"--help")
                help_flag=1
                shift;;
            "-u"|"--users")
                users=1
                shift;;
            "-p"|"--processes")
                processes=1
                shift;;
            "-l"|"--log")
                out_path="$2"
                shift;;
            "-e"|"--errors")
                err_path="$2"
                shift;;
            "-"|"--")
                shift;;
            "*")
                echo "Unknown argument $1"
                exit 1;;
        esac
        shift
    done
}

get_users() {
    local users=$($cat /etc/passwd | $sort | $cut -d : -f 1,6)

    printf "%-32s%s\n" "USER" "HOME"
    for line in $users
    do
        printf "%-32s%s\n" $($sed "s/:/\n/g" <<< $line)
    done
}

get_processes() {
    echo 0
}

create_file() {
    
}

###################################
#             Entry               #
###################################
parse_arguments $@

if [ -n "$out_path" ]; then
    exec 1>$out_path
fi

if [ -n "$err_path" ]; then
    exec 2>$err_path
fi


if [ $help_flag -eq 1 ]; then
    show_help
elif [ $users -eq 1 ]; then
    get_users
elif [ $processes -eq 1 ]; then
    get_processes
fi
exit 0
