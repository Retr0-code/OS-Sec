#pragma once
#include "status.hpp"

enum network_exceptions
{
    socket_error_success,
    socket_error_init,
    socket_error_bind,
    socket_error_listen,
    socket_error_domain,
    socket_error_connect,
    socket_error_transmission
};
