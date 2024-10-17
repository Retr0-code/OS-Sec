#include "status.hpp"
#include "client/client.hpp"
#include "network_exceptions.hpp"

Client::Client(const char *lhost, in_port_t lport, bool use_ipv6)
    : _use_ipv6(use_ipv6),
    _address_ipv4({AF_INET, 0, 0}),
    _address_ipv6({AF_INET6, 0, 0, 0, 0})
{
    sa_family_t domain = AF_INET;
    if (this->_use_ipv6)
        domain = AF_INET6;
    
    this->_socket_descriptor = ::socket(domain, SOCK_STREAM, 0);
    if (this->_socket_descriptor < 0)
        throw socket_init_error(errno);

    typedef sockaddr* (Client::*init_ptr)(const char*, in_port_t);
    init_ptr bind_func = &Client::init_ipv4;
    if (this->_use_ipv6)
        bind_func = &Client::init_ipv6;
    
    std::invoke(bind_func, this, lhost, lport);
}

Client::~Client()
{
    this->close_connection();
}

void Client::connect(void)
{
    sockaddr* server{reinterpret_cast<sockaddr*>(&this->_address_ipv4)};
    if (this->_use_ipv6)
        server = reinterpret_cast<sockaddr*>(&this->_address_ipv6);
    
    if (::connect(this->_socket_descriptor, server, sizeof(*server)) < 0)
        throw socket_connect_error{errno};

    std::cout << SUCCESS << " Client successfully connected to the host\n";
}

void Client::close_connection(void)
{
    if (::shutdown(this->_socket_descriptor, SHUT_RDWR) != 0)
        std::cerr << ERROR << "Shutdown Client:\t" << std::strerror(errno);
    
    if (::close(this->_socket_descriptor) != 0)
        std::cerr << ERROR << "Closing Client:\t" << std::strerror(errno);
}

size_t Client::write(const uint8_t *data, size_t size)
{
    size_t sent{::send(this->_socket_descriptor, data, size, 0)};
    if (sent == -1)
        throw transmission_error(errno, this->_socket_descriptor, transmission_error::write);

    return sent;
}

size_t Client::read(uint8_t *data, size_t size)
{
    size_t received{::recv(this->_socket_descriptor, data, size, 0)};
    if (received == -1)
        throw transmission_error(errno, this->_socket_descriptor, transmission_error::read);

    return received;
}

sockaddr *Client::init_ipv4(const char *lhost, in_port_t lport)
{
    this->_address_ipv4.sin_family = AF_INET;
    this->_address_ipv4.sin_port = ::htons(lport);
    if (::inet_pton(AF_INET, lhost, &this->_address_ipv4.sin_addr) != 1)
        throw std::invalid_argument(lhost);

    return reinterpret_cast<sockaddr*>(&this->_address_ipv4);
}

sockaddr *Client::init_ipv6(const char* lhost, in_port_t lport)
{
    std::string s_lhost {lhost};
    std::string ipv6 {s_lhost.substr(0, s_lhost.find_last_of('%'))};
    std::string scope_id {s_lhost.substr(s_lhost.find_last_of('%') + 1)};

    this->_address_ipv6.sin6_family = AF_INET6;
    this->_address_ipv6.sin6_port = ::htons(lport);

    if (::inet_pton(AF_INET6, ipv6.c_str(), &this->_address_ipv6.sin6_addr) != 1)
        throw std::invalid_argument(lhost);
    
    try
    {
        this->_address_ipv6.sin6_scope_id = std::stoi(scope_id);
    }
    catch (std::invalid_argument const& exception)
    {
#if DEBUG
        std::cerr << WARNING << " Please use scope id rather then interface name\n";
#endif
        this->_address_ipv6.sin6_scope_id = this->get_scope_id(scope_id.c_str());
    }

    return reinterpret_cast<sockaddr*>(&_address_ipv6);
}

inline uint32_t Client::get_scope_id(const char* interface_name)
{
    ifreq interface_descriptor;
    interface_descriptor.ifr_addr.sa_family = AF_INET;
    std::strncpy(interface_descriptor.ifr_name, interface_name, IFNAMSIZ - 1);

    // Get interface general info
    ::ioctl(this->_socket_descriptor, SIOCGIFADDR, &interface_descriptor);
    return interface_descriptor.ifr_ifru.ifru_ivalue;
}
