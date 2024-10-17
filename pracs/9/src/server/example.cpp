#include "server/server.hpp"
#include "network_exceptions.hpp"

#ifdef USE_IPv6
#define HOST    "::1%lo"
#else
#define HOST    "127.0.0.1"
#endif
#define PORT    4645

#define BUFFER_SIZE 4096

int main()
{
    std::shared_ptr<Server> server;
    try
    {
        server = Server::instance(HOST, PORT
#ifdef USE_IPv6
, true
#endif
    );
    }
    catch(const network_exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    server->listen();
    bool just_started{true};

    char msg[BUFFER_SIZE];
    
    while (just_started || server->clients_amount())
    {
        if (server->clients_amount() > 0)
        {
            just_started = false;
            if ((*server)[0]->read(reinterpret_cast<uint8_t*>(msg), BUFFER_SIZE) != 0)
                std::cout << "Client sent message: " << msg << '\n';

            if (std::strncmp("close", msg, 6) == 0)
                (*server)[0]->close_connection();
        }
    }

    return 0;
}
