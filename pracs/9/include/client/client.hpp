#pragma once
// STD headers
#include <memory>
#include <iostream>
#include <functional>

// Net linux headers
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

class Client
{
public:
    explicit Client(const char* lhost, in_port_t lport, bool use_ipv6 = false);
    
    ~Client();

    void connect(void);
    
    void close_connection(void);

    size_t write(const uint8_t* data, size_t size);

    size_t read(uint8_t* data, size_t size);

private:
    sockaddr* init_ipv4(const char* lhost, in_port_t lport);
    
    sockaddr* init_ipv6(const char* lhost, in_port_t lport);

    uint32_t get_scope_id(const char * interface_name);

private:
    bool _use_ipv6;
    int _socket_descriptor;     // Client filedescriptor
    sockaddr_in _address_ipv4;  // Descriptor for IPv4 configuration
    sockaddr_in6 _address_ipv6; // Descriptor for IPv6 configuration
};
