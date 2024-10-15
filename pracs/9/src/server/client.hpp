#pragma once
#include <memory>
#include <vector>
#include <cstring>
#include <iostream>

#include <unistd.h>
#include <sys/socket.h>

#define DEFAULT_BUFFER_SIZE 0x1000

class Server;

class Client
{
// Public methods
public:
    Client(int sock_fd, std::shared_ptr<Server> const server_instanse);

    ~Client();

    static void set_buffer_size(size_t size);

    uint32_t get_id(void) const;

    void close_connection(void);

    size_t operator<<(const std::vector<uint8_t>& data);
    
// Private member variables
private:
    static uint32_t _clients_amount;
    static size_t _buffer_size;
    uint32_t _client_id;
    int32_t _socket_descriptor;
    std::shared_ptr<Server> _server_instanse;
    std::vector<uint8_t> _transmission_buffer;
};
