#pragma once

// STD headers
#include <stdio.h>
#include <string.h>
#include <threads.h>
#include <stdatomic.h>

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
#include "server/client.h"
#include "hashtable/hashtable.h"

/*
    Server class provided to ditribute threads to remote clients.
    Only one instanse of this class is avalible for using. If you
    try to create one more you would get exception
    "server_instance_error" with address of object provided.
*/
typedef struct
{
    uint16_t        _clients_max_amount;    // By default is 1000
    int             _socket_descriptor;     // Descriptor of server socket
    thrd_t          _listener;              // Listener thread
    atomic_int      _stop_listening;        // State variable for listening thread
    sockaddr        _address;               // Address descriptor
    hashtable_t     _clients;               // Pointers to clients
} Server;

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
    int Server_create(
        Server *server,
        const char* lhost,
        in_port_t lport,
        bool use_ipv6 = false,
        uint16_t clients_max_amount = 100
    );
    
    void Server_close(Server *server);

    /*  Runs detached thread that accepts new clientreturns its id

        Returns thread's id

        Exceptions:
         * all exceptions of "thread( Function&& f, Args&&... args )" constructor
    */
    thrd_t Server_listen(void);

    void Server_disconnect(uint32_t client_id);
    
    ClientInterface* Server_get(uint32_t id);

    size_t Server_clients_amount(void);

    const hashtable* Server_clients(void);
