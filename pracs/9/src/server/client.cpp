#include "status.hpp"
#include "server/client.hpp"
#include "server/server.hpp"
#include "network_exceptions.hpp"

uint32_t ClientInterface::_clients_amount{0};

ClientInterface::ClientInterface(int sock_fd, std::shared_ptr<Server> const server_instanse) :
    _socket_descriptor(sock_fd), _server_instanse(server_instanse), _client_id(_clients_amount++) {  }

ClientInterface::~ClientInterface()
{
    if (::shutdown(this->_socket_descriptor, SHUT_RDWR) != 0)
        std::cerr << ERROR << "Shutdown Client:\t" << std::strerror(errno);
    
    if (::close(this->_socket_descriptor) != 0)
        std::cerr << ERROR << "Closing Client:\t" << std::strerror(errno);

    this->_server_instanse->disconnect(this->_client_id);
}

uint32_t ClientInterface::get_id(void) const
{
    return this->_client_id;
}

void ClientInterface::close_connection(void)
{
    this->_server_instanse->disconnect(this->_client_id);
}

size_t ClientInterface::write(const uint8_t *data, size_t size)
{
    size_t sent{::write(this->_socket_descriptor, data, size)};
    if (sent == -1)
    {
        sent = 0;
        throw transmission_error(errno, this->_socket_descriptor, transmission_error::write);
    }

    return sent;
}

size_t ClientInterface::read(uint8_t *data, size_t size)
{
    size_t received{::read(this->_socket_descriptor, data, size)};
    if (received == -1)
        throw transmission_error(errno, this->_socket_descriptor, transmission_error::read);

    return received;
}
