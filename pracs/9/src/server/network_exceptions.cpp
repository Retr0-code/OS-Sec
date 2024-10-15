#include "network_exceptions.hpp"

server_instance_error::server_instance_error(void* address) : _error_address(address) { }

const char* server_instance_error::what() const throw()
{
    std::ostringstream stream_error;
    stream_error << ERROR << " Only one instance of class Server can exist " << std::hex << _error_address << std::endl;

#if DEBUG
    std::cout << stream_error.str();
#endif

    return stream_error.str().c_str();
}

void* server_instance_error::get_address()
{
    return _error_address;
}


socket_init_error::socket_init_error(int err_code, void* err_address) :
    _error_code(err_code), _error_address(err_address) { }

const char* socket_init_error::what() const throw()
{
    std::ostringstream stream_error;
    stream_error << ERROR " Unable to run server: " <<
    std::hex << _error_address << "\terror: " << std::strerror(_error_code) << std::endl;

#if DEBUG
    std::cout << stream_error.str();
#endif

    return stream_error.str().c_str();
}

void* socket_init_error::get_address()
{
    return _error_address;
}


socket_bind_error::socket_bind_error(int err_code, const char* ip, uint16_t port, void* err_address) :
    _error_code(err_code), _ip(ip), _port(port), _error_address(err_address) { }

const char* socket_bind_error::what() const throw()
{
    std::ostringstream stream_error;
    stream_error << ERROR << " Unable to bind server " << _ip << ':'
    << _port << std::endl << std::hex << _error_address << "\terror: "
    << std::strerror(_error_code) << std::endl;

#if DEBUG
    std::cout << stream_error.str();
#endif

    return stream_error.str().c_str();
}

void* socket_bind_error::get_address()
{
    return _error_address;
}


socket_listen_error::socket_listen_error(int err_code) : _error_code(err_code) { }

const char* socket_listen_error::what() const throw()
{
    std::ostringstream stream_error;
    stream_error << ERROR << " Unable to listen " << 
    "\terror: " << std::strerror(_error_code) << std::endl;

#if DEBUG
    std::cout << stream_error.str();
#endif

    return stream_error.str().c_str();
}

unsupported_domain_type::unsupported_domain_type(uint16_t domain) : _domain(domain) { }

const char* unsupported_domain_type::what() const throw()
{
    std::ostringstream stream_error;
    stream_error << ERROR << " Unsupported domain type " << _domain << std::endl;

#if DEBUG
    std::cout << stream_error.str();
#endif

    return stream_error.str().c_str();
}

uint16_t unsupported_domain_type::get_domain()
{
    return _domain;
}
