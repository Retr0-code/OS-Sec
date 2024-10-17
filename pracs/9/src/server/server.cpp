#include <iomanip>

#include "server/server.hpp"
#include "status.hpp"
#include "network_exceptions.hpp"

bool Server::_is_instantiated{false};
std::shared_ptr<Server> Server::_server_instance{nullptr};

Server::Server(
    const char* lhost,
    in_port_t lport,
    bool use_ipv6,
    uint16_t clients_max_amount)
    : _use_ipv6(use_ipv6),
    _clients_max_amount(clients_max_amount),
    _address_ipv4({AF_INET, 0, 0}),
    _address_ipv6({AF_INET6, 0, 0, 0, 0})
{
    sa_family_t domain = AF_INET;
    if (this->_use_ipv6)
        domain = AF_INET6;
    
    this->_socket_descriptor = ::socket(domain, SOCK_STREAM, 0);
    if (this->_socket_descriptor < 0)
        throw socket_init_error(errno);

    typedef sockaddr* (Server::*bind_ptr)(const char*, in_port_t);
    
    bind_ptr bind_func = &Server::bind_ipv4;
    if (this->_use_ipv6)
        bind_func = &Server::bind_ipv6;
    
    std::invoke(bind_func, this, lhost, lport);

    this->_clients.reserve(this->_clients_max_amount);
    
    Server::_is_instantiated = true;

#if DEBUG
    if (this->_use_ipv6)
        std::cout << INFO << " IPv6:\t" << std::hex << std::setfill('0') <<
        std::setw(32) << this->_address_ipv6.sin6_addr << ':' << std::setw(4) << this->_address_ipv6.sin6_port << std::endl;
    else
        std::cout << INFO << " IPv4:\t" << std::hex << std::setfill('0') <<
        std::setw(32) << this->_address_ipv4.sin6_addr << ':' << std::setw(4) << this->_address_ipv4.sin6_port << std::endl;

    std::cout << INFO << " Submited\t" << lhost<< ":" << lport << std::endl;
#endif
}

std::shared_ptr<Server> Server::instance(void)
{
    if (Server::_is_instantiated)
        return Server::_server_instance;
    
    throw server_instance_error();
}

std::shared_ptr<Server> Server::instance(
    const char *lhost,
    in_port_t lport,
    bool use_ipv6,
    uint16_t clients_max_amount)
{
    if (!Server::_is_instantiated)
        Server::_server_instance = std::shared_ptr<Server>(new Server(lhost, lport, use_ipv6, clients_max_amount));

    return _server_instance;
}

Server::~Server()
{
    this->Stop();
    
    if (::shutdown(this->_socket_descriptor, SHUT_RDWR) != 0)
        std::cerr << WARNING << " Shutdown Server:\t\n" << std::strerror(errno) << std::endl;

    if (::close(this->_socket_descriptor) != 0)
        std::cerr << WARNING << " Closing Server:\t\n" << std::strerror(errno) << std::endl;
}

sockaddr* Server::bind_ipv4(const char* lhost, in_port_t lport)
{
    this->_address_ipv4.sin_family = AF_INET;
    this->_address_ipv4.sin_port = ::htons(lport);
    if (::inet_pton(AF_INET, lhost, &this->_address_ipv4.sin_addr) != 1)
        throw std::invalid_argument(lhost);

    if (::bind(
        this->_socket_descriptor,
        reinterpret_cast<sockaddr*>(&this->_address_ipv4),
        sizeof(this->_address_ipv4)
        ) != 0)
        throw socket_bind_error(errno, lhost, lport);

    return reinterpret_cast<sockaddr*>(&this->_address_ipv4);
}

sockaddr* Server::bind_ipv6(const char* lhost, in_port_t lport)
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

    if (::bind(this->_socket_descriptor, reinterpret_cast<sockaddr*>(&_address_ipv6), sizeof(_address_ipv6)) != 0)
        throw socket_bind_error(errno, lhost, lport);

    return reinterpret_cast<sockaddr*>(&_address_ipv6);
}

uint32_t Server::get_scope_id(const char * interface_name)
{
    ifreq interface_descriptor;
    interface_descriptor.ifr_addr.sa_family = AF_INET;
    std::strncpy(interface_descriptor.ifr_name, interface_name, IFNAMSIZ - 1);

    // Get interface general info
    ::ioctl(this->_socket_descriptor, SIOCGIFADDR, &interface_descriptor);
    return interface_descriptor.ifr_ifru.ifru_ivalue;
}

std::thread::id Server::listen()
{
    _listener = std::thread(&Server::listen_connection, this);
    _listener.detach();
    return _listener.get_id();
}

int Server::listen_connection()
{
    int listen_status {0};
    this->_stop_listening = false;

    std::cout << INFO << " Listening for new connections ...\n";

    while (!this->_stop_listening)
    {
        listen_status = ::listen(_socket_descriptor, 4);
        if (listen_status < 0)
            throw socket_listen_error(errno);

        if (this->_clients.size() <= _clients_max_amount && listen_status == 0)
            this->accept_client();
    }

    return listen_status;
}

void Server::accept_client()
{
    sockaddr* address {reinterpret_cast<sockaddr*>(&_address_ipv4)};
    if (this->_use_ipv6)
        address = reinterpret_cast<sockaddr*>(&_address_ipv6);

    socklen_t socket_length {sizeof(*address)};
    int new_client_socket {::accept(_socket_descriptor, address, &socket_length)};
    if (new_client_socket < 0)
        throw socket_listen_error(errno);

    std::cout << SUCCESS << " Accepting new client " << new_client_socket - _socket_descriptor << std::endl;
    std::shared_ptr<ClientInterface> new_client = std::make_shared<ClientInterface>(new_client_socket, Server::_server_instance);

    this->_clients.emplace(new_client->get_id(), new_client);
    std::cout << INFO << " Connected new client " << new_client->get_id() << std::endl;
}

void Server::disconnect(uint32_t client_id)
{
    this->_clients.erase(client_id);
    std::cout << INFO << " Client " << client_id << " has disconnected\n";
}

void Server::Stop(void)
{
    std::cout << INFO << " Stopping server's listener...\n";
    this->_stop_listening = true;
}

const std::shared_ptr<ClientInterface> &Server::operator[](uint32_t id) const
{
    return this->_clients.at(id);
}

std::shared_ptr<ClientInterface> &Server::operator[](uint32_t id)
{
    return this->_clients[id];
}

size_t Server::clients_amount(void) const
{
    return this->_clients.size();
}

std::unordered_map<uint32_t, std::shared_ptr<ClientInterface>> &Server::clients(void)
{
    return this->_clients;
}
