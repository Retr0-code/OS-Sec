#pragma once
#include <cstring>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <exception>

#include "color_codes.hpp"

//  server_instance_error raises in case of attempting to create new Server object
class server_instance_error : public std::exception
{
public:
    // Takes address of server object
    explicit server_instance_error(void* address);

    const char* what() const throw() override;

    // Returns address of Server object
    void* get_address();

private:
    void* _error_address;
};

class socket_init_error : public std::exception
{
public:
    explicit socket_init_error(int err_code, void* err_address);

    const char* what() const throw() override;

    // Returns address of Server object
    void* get_address();

private:
    int _error_code;
    void* _error_address;
};

class socket_bind_error : public std::exception
{
public:
    explicit socket_bind_error(int err_code, const char* ip, uint16_t port, void* err_address);

    const char* what() const throw() override;

    // Returns address of Server object
    void* get_address();

private:
    int _error_code;
    uint16_t _port;
    void* _error_address;
    const char* _ip;
};

class socket_listen_error : public std::exception
{
public:
    explicit socket_listen_error(int err_code);

    const char* what() const throw() override;

private:
    int _error_code;
};

class unsupported_domain_type : public std::exception
{
public:
    explicit unsupported_domain_type(uint16_t domain);

    const char* what() const throw() override;

    // Returns unsopported domain
    uint16_t get_domain();

private:
    uint16_t _domain;
};
