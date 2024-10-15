#include "client.hpp"
#include "server.hpp"
#include "color_codes.hpp"

uint32_t Client::_clients_amount{0};
size_t Client::_buffer_size{DEFAULT_BUFFER_SIZE};

Client::Client(int sock_fd, std::shared_ptr<Server> const server_instanse) :
    _socket_descriptor(sock_fd), _server_instanse(server_instanse), _client_id(_clients_amount++) {  }

Client::~Client()
{
    if (::shutdown(this->_socket_descriptor, SHUT_RDWR) != 0)
        std::cerr << ERROR << "Shutdown Client:\t" << std::strerror(errno);
    
    if (::close(this->_socket_descriptor) != 0)
        std::cerr << ERROR << "Closing Client:\t" << std::strerror(errno);
}

void Client::set_buffer_size(size_t size)
{
    Client::_buffer_size = size;
}

uint32_t Client::get_id(void) const
{
    return this->_client_id;
}

void Client::close_connection(void)
{
    this->_server_instanse->disconnect(this);
}

size_t Client::operator<<(const std::vector<uint8_t> &data)
{
    if (data.empty())
        return 0;

    size_t sent{::send(this->_socket_descriptor, data.data(), data.size(), 0)};
    if (sent != data.size())
        std::cerr << WARNING << "Client " << this->_client_id << " Failed to send all data of size "
        << data.size() << " from "<< std::hex << data.data() << std::endl;

    else if (sent == -1)
    {
        sent = 0;
        std::cerr << ERROR << "Client " << this->_client_id << " Failed to send data to client " << std::endl;
    }

    return sent;
}

// const std::vector<uint8_t>& operator<<(Client& client)
// {
//     size_t received{::recv(this->_socket_descriptor, data.data(), Client::_buffer_size, 0)};
//     if (received != data.size())
//         std::cerr << WARNING << "Client " << this->_client_id << " Failed to receive all data of size "
//         << data.size() << " from "<< std::hex << data.data() << std::endl;

//     else if (received == -1)
//     {
//         received = 0;
//         std::cerr << ERROR << "Client " << this->_client_id << " Failed to receive data from client " << std::endl;
//     }

//     return received;
// }
