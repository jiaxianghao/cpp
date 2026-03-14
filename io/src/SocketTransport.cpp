#include "SocketTransport.h"
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>

namespace transport
{

SocketTransport::SocketTransport(const SocketConfig& config)
    : m_config(config)
    , m_clientFd(-1)
{
}

SocketTransport::~SocketTransport()
{
    Close();
}

bool SocketTransport::Open()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_connected)
    {
        SetError("Socket already open");
        return false;
    }

    bool result = false;
    if (m_config.protocol == SocketConfig::Protocol::TCP)
    {
        result = SetupTCP();
    }
    else
    {
        result = SetupUDP();
    }

    if (result)
    {
        SetConnected(true);
    }

    return result;
}

void SocketTransport::Close()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_clientFd >= 0)
    {
        ::close(m_clientFd);
        m_clientFd = -1;
    }

    if (m_fd >= 0)
    {
        ::close(m_fd);
        m_fd = -1;
    }

    SetConnected(false);
}

int SocketTransport::Send(const void* data, size_t size)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_connected)
    {
        SetError("Socket not connected");
        return -1;
    }

    int sock = (m_config.is_server && m_clientFd >= 0) ? m_clientFd : m_fd;
    if (sock < 0)
    {
        SetError("Invalid socket descriptor");
        return -1;
    }

    ssize_t result = ::send(sock, data, size, 0);
    if (result < 0)
    {
        SetErrorFromErrno("Failed to send data");
        return -1;
    }

    return static_cast<int>(result);
}

int SocketTransport::Receive(void* buffer, size_t size)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_connected)
    {
        SetError("Socket not connected");
        return -1;
    }

    int sock = (m_config.is_server && m_clientFd >= 0) ? m_clientFd : m_fd;
    if (sock < 0)
    {
        SetError("Invalid socket descriptor");
        return -1;
    }

    ssize_t result = ::recv(sock, buffer, size, 0);
    if (result < 0)
    {
        SetErrorFromErrno("Failed to receive data");
        return -1;
    }

    return static_cast<int>(result);
}

TransportType SocketTransport::GetType() const
{
    return TransportType::SOCKET;
}

bool SocketTransport::SetupTCP()
{
    // Create socket
    m_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd < 0)
    {
        SetErrorFromErrno("Failed to create TCP socket");
        return false;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        SetErrorFromErrno("Failed to set socket options");
        ::close(m_fd);
        m_fd = -1;
        return false;
    }

    struct sockaddr_in addr;
    if (!ResolveAddress(m_config.address, addr))
    {
        ::close(m_fd);
        m_fd = -1;
        return false;
    }

    addr.sin_port = htons(m_config.port);

    if (m_config.is_server)
    {
        // Server mode: bind and listen
        if (::bind(m_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        {
            SetErrorFromErrno("Failed to bind TCP socket");
            ::close(m_fd);
            m_fd = -1;
            return false;
        }

        if (::listen(m_fd, 5) < 0)
        {
            SetErrorFromErrno("Failed to listen on TCP socket");
            ::close(m_fd);
            m_fd = -1;
            return false;
        }

        // Accept a client connection
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        m_clientFd = ::accept(m_fd, (struct sockaddr*)&client_addr, &client_len);
        if (m_clientFd < 0)
        {
            SetErrorFromErrno("Failed to accept client connection");
            ::close(m_fd);
            m_fd = -1;
            return false;
        }
    }
    else
    {
        // Client mode: connect
        if (::connect(m_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        {
            SetErrorFromErrno("Failed to connect to TCP server");
            ::close(m_fd);
            m_fd = -1;
            return false;
        }
    }

    return true;
}

bool SocketTransport::SetupUDP()
{
    // Create socket
    m_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (m_fd < 0)
    {
        SetErrorFromErrno("Failed to create UDP socket");
        return false;
    }

    struct sockaddr_in addr;
    if (!ResolveAddress(m_config.address, addr))
    {
        ::close(m_fd);
        m_fd = -1;
        return false;
    }

    addr.sin_port = htons(m_config.port);

    if (m_config.is_server)
    {
        // Server mode: bind to address
        if (::bind(m_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        {
            SetErrorFromErrno("Failed to bind UDP socket");
            ::close(m_fd);
            m_fd = -1;
            return false;
        }
    }
    else
    {
        // Client mode: connect to remote address (optional for UDP, but simplifies send/recv)
        if (::connect(m_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        {
            SetErrorFromErrno("Failed to connect UDP socket");
            ::close(m_fd);
            m_fd = -1;
            return false;
        }
    }

    return true;
}

bool SocketTransport::ResolveAddress(const std::string& address, struct sockaddr_in& addr)
{
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    // Try to parse as IP address first
    if (inet_pton(AF_INET, address.c_str(), &addr.sin_addr) == 1)
    {
        return true;
    }

    // Try to resolve as hostname
    struct hostent* host = gethostbyname(address.c_str());
    if (host == nullptr)
    {
        SetError("Failed to resolve hostname: " + address);
        return false;
    }

    std::memcpy(&addr.sin_addr, host->h_addr_list[0], host->h_length);
    return true;
}

} // namespace transport
