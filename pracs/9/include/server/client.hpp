#pragma once
#include <memory>
#include <vector>
#include <cstring>
#include <iostream>

#include <unistd.h>
#include <sys/socket.h>

#define DEFAULT_BUFFER_SIZE 0x1000

class Server;

class ClientInterface
{
// Public methods
public:
    ClientInterface(int sock_fd, std::shared_ptr<Server> const server_instanse);

    ~ClientInterface();

    uint32_t get_id(void) const;

    void close_connection(void);

    size_t write(const uint8_t* data, size_t size);

    size_t read(uint8_t* data, size_t size);
    
// Private member variables
private:
    static uint32_t _clients_amount;
    uint32_t _client_id;
    int32_t _socket_descriptor;
    std::shared_ptr<Server> _server_instanse;
};
