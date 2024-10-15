#pragma once

// STD headers
#include <memory>
#include <future>
#include <atomic>
#include <thread>
#include <vector>
#include <cstring>
#include <iostream>
#include <functional>
#include <unordered_map>

// Net linux headers
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Own headers
#include "client.hpp"

/*
    Server class provided to ditribute threads to remote clients.
    Only one instanse of this class is avalible for using. If you
    try to create one more you would get exception
    "server_instance_error" with address of object provided.
*/
class Server
{
// Public methods
private:
/*  Creates Server instanse with parameters:

    Parameters:
     * const char* lhost - local IPv4 or IPv6
     * in_port_t lport - port to run service on
     * bool use_ipv6 - specifies protocol version IPv6 (by default is false)
     * uint16_t clients_max_amount - specifies maximum of clients connected to the server

    Exceptions:
     * server_instance_error
     * socket_init_error
     * socket_bind_error
*/
    Server(
        const char* lhost,
        in_port_t lport,
        bool use_ipv6 = false,
        uint16_t clients_max_amount = 1000
    );

public:
    static std::shared_ptr<Server> instance(void);

    static std::shared_ptr<Server> instance(
        const char* lhost,
        in_port_t lport,
        bool use_ipv6 = false,
        uint16_t clients_max_amount = 1000
    );

    ~Server();

    /*  Runs detached thread that accepts new clientreturns its id

        Returns thread's id

        Exceptions:
         * all exceptions of "thread( Function&& f, Args&&... args )" constructor
    */
    std::thread::id listen(void);

    void disconnect(Client* client_instanse);

    // Stops listening
    void Stop(void);

    // For singleton like implementation deleted all copy providers
    Server(void) = delete;

    Server(const Server&) = delete;
    
    Server(Server&&) = delete;
    
    Server operator=(Server) = delete;
    
    Server operator=(Server&&) = delete;
    
    Server operator=(const Server&) = delete;

    const std::shared_ptr<Client>& operator[](uint32_t id) const;
    
    std::shared_ptr<Client>& operator[](uint32_t id);
    
    void operator<<(const std::vector<uint8_t>& data);
    
    void operator>>(const std::vector<uint8_t>& data);

// Private methods
private:
    int listen_connection(void);

    inline sockaddr* bind_ipv4(const char* lhost, in_port_t lport);
    
    inline sockaddr* bind_ipv6(const char* lhost, in_port_t lport);

    inline uint32_t get_scope_id(const char * interface_name);

    void accept_client(void);

// Private member variables
private:
    static bool _is_instantiated;       // Stores "true" if the object already exists (for singleton like implementation)
    bool _use_ipv6;                     // By default is false
    int _socket_descriptor;             // Descriptor of server socket
    std::atomic_bool _stop_listening;   // State variable for listening thread
    sockaddr_in _address_ipv4;          // Descriptor for IPv4 configuration
    sockaddr_in6 _address_ipv6;         // Descriptor for IPv6 configuration
    uint16_t _clients_max_amount;       // By default is 1000
    std::thread _listener;              // Listener thread
    std::unordered_map<uint32_t, std::shared_ptr<Client>> _clients;      // Pointers to clients
    // std::vector<std::shared_ptr<Client>> _clients;      // Pointers to clients
    // std::vector<std::future<void>> _clients_threads;    // Async clients list
    static std::shared_ptr<Server> _server_instance;    // Singletone instance
};
