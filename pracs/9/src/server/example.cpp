#include "server.hpp"
#include "network_exceptions.hpp"

#ifdef USE_IPv6
#define HOST    "::1%lo"
#else
#define HOST    "127.0.0.1"
#endif
#define PORT    4645

int main()
{
    std::shared_ptr<Server> server = Server::instance(HOST, PORT
#ifdef USE_IPv6
, true
#endif
);
    server->listen();
    std::cin.get();
    (*server)[1] << 

    return 0;
}
