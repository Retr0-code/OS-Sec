#pragma once
#include <cstring>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <exception>

#include "status.hpp"

#define EXCEPTION_BUFFER_LENGTH 256

class network_exception : public std::exception
{
public:
    network_exception(void) = delete;

    virtual const char* what() const throw()
    {
        return _message;
    }

protected:
    template <typename... T>
    network_exception(const std::string& format = "", T... args)
    {
        std::string finalFormat = ERROR + format;
        std::sprintf(_message, finalFormat.c_str(), args...);
    }

protected:
    char _message[EXCEPTION_BUFFER_LENGTH];
};

//  server_instance_error raises in case of attempting to create new Server object
class server_instance_error : public network_exception
{
public:
    // Takes address of server object
    explicit server_instance_error(void);
};

class socket_init_error : public network_exception
{
public:
    explicit socket_init_error(int err_code);

private:
    int _error_code;
    void* _error_address;
};

class socket_bind_error : public network_exception
{
public:
    explicit socket_bind_error(int err_code, const char* ip, uint16_t port);

private:
    int _error_code;
    uint16_t _port;
    const char* _ip;
};

class socket_listen_error : public network_exception
{
public:
    explicit socket_listen_error(int err_code);

private:
    int _error_code;
};

class unsupported_domain_type : public network_exception
{
public:
    explicit unsupported_domain_type(uint16_t domain);

    // Returns unsopported domain
    uint16_t get_domain();

private:
    uint16_t _domain;
};

class socket_connect_error : public network_exception
{
public:
    explicit socket_connect_error(int err_code);

private:
    int _error_code;
};

class transmission_error : public network_exception
{
public:
    enum socket_operation
    {
        read,
        write
    };

    transmission_error(int err_code, int socket, socket_operation op);

private:
    int _error_code;
};
