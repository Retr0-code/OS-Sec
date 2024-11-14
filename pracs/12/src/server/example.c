#include "server/server.h"
#include "network_exceptions.h"

#ifdef USE_IPv6
#define HOST    "::1%lo"
#else
#define HOST    "127.0.0.1"
#endif
#define PORT    4645

#define BUFFER_SIZE 4096

int main()
{
    Server *server = malloc(sizeof(Server));
    Server_create(server, HOST, PORT, 0, 100);

    Server_listen(server);

    int just_started = 0;
    char msg[BUFFER_SIZE];
    while (just_started || server->_clients_amount)
    {
        if (server->_clients_amount > 0)
        {
            just_started = 0;
            memset(msg, 0, BUFFER_SIZE);
            if (read(server->_clients[0]->_socket_descriptor, msg, BUFFER_SIZE) != 0)
                printf("Client sent message: %s\n", msg);

            if (strncmp("close", msg, 6) == 0)
                Server_close_connection(server->_clients[0]);
        }
    }

    return 0;
}
