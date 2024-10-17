#include "network_exceptions.hpp"

server_instance_error::server_instance_error(void)
    : network_exception("Server or client is not instantiated\n") {  }

socket_init_error::socket_init_error(int err_code)
    : _error_code(err_code),
    network_exception("Unable to intialize a socket:\n%s\n", std::strerror(err_code)) {  }

socket_bind_error::socket_bind_error(int err_code, const char* ip, uint16_t port)
    : _error_code(err_code), _ip(ip), _port(port),
    network_exception("Unable to bind socket %s:%i:\n%s\n", ip, port, std::strerror(err_code)) {  }

socket_listen_error::socket_listen_error(int err_code)
    : _error_code(err_code),
    network_exception("Unable to start listener:\n%s\n", std::strerror(err_code)) {  }

unsupported_domain_type::unsupported_domain_type(uint16_t domain)
    : _domain(domain),
    network_exception("Unsupported domain type %i\n", domain) {  }

uint16_t unsupported_domain_type::get_domain()
{
    return _domain;
}

socket_connect_error::socket_connect_error(int err_code)
    : _error_code(err_code),
    network_exception("Unable to connect to the server:\n%s\n", std::strerror(err_code)) {  }

transmission_error::transmission_error(int err_code, int socket, socket_operation op)
    : _error_code(err_code),
    network_exception("Unable to %s from socket %i:\n%s\n", op == read ? "read" : "write", socket, std::strerror(err_code)) {  }
