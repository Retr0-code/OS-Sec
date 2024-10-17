#include <iostream>
#include "client/client.hpp"

#ifdef USE_IPv6
#define HOST    "::1%lo"
#else
#define HOST    "127.0.0.1"
#endif
#define PORT    4645

int main()
{
    Client client(HOST, PORT
#ifdef USE_IPv6
, true
#endif
    );
    client.connect();

    std::string input;
    while (input != "close")
    {
        std::cin >> input;
        client.write(reinterpret_cast<const uint8_t*>(input.c_str()), input.size());
    }

    return 0;
}