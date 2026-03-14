#include "network_lib/tcp_socket.h"
#include <cstring>
#include <stdexcept>

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#endif

namespace network_lib
{

TCPSocket::TCPSocket()
    : socket_(INVALID_SOCKET), connected_(false), listening_(false)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    initializeSocket();
}

TCPSocket::~TCPSocket()
{
    close();
#ifdef _WIN32
    WSACleanup();
#endif
}

void TCPSocket::initializeSocket()
{
    socket_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ == INVALID_SOCKET)
    {
        throw std::runtime_error("Failed to create TCP socket");
    }
}

void TCPSocket::cleanupSocket()
{
    if (socket_ != INVALID_SOCKET)
    {
#ifdef _WIN32
        closesocket(socket_);
#else
        ::close(socket_);
#endif
        socket_ = INVALID_SOCKET;
    }
    connected_ = false;
    listening_ = false;
}

void TCPSocket::setSocket(SOCKET socket)
{
    if (socket_ != INVALID_SOCKET && socket_ != socket)
    {
        cleanupSocket();
    }
    socket_ = socket;
    connected_ = true;
}

bool TCPSocket::connect(const std::string& host, int port)
{
    if (connected_)
    {
        close();
        initializeSocket();
    }

    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    // Convert hostname to IP address
    if (inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0)
    {
        // Try to resolve hostname
        struct hostent* hostEntry = gethostbyname(host.c_str());
        if (hostEntry == nullptr)
        {
            return false;
        }
        std::memcpy(&serverAddr.sin_addr, hostEntry->h_addr_list[0], hostEntry->h_length);
    }

    if (::connect(socket_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        return false;
    }

    connected_ = true;
    return true;
}

bool TCPSocket::send(const void* data, size_t len)
{
    if (!connected_)
    {
        return false;
    }

    const char* buffer = static_cast<const char*>(data);
    size_t totalSent = 0;

    while (totalSent < len)
    {
        int sent = ::send(socket_, buffer + totalSent, len - totalSent, 0);
        if (sent == SOCKET_ERROR)
        {
            connected_ = false;
            return false;
        }
        totalSent += sent;
    }

    return true;
}

bool TCPSocket::receive(void* buffer, size_t& len)
{
    if (!connected_)
    {
        return false;
    }

    size_t bufferSize = len;
    int received = ::recv(socket_, static_cast<char*>(buffer), bufferSize, 0);

    if (received == SOCKET_ERROR || received == 0)
    {
        connected_ = false;
        return false;
    }

    len = received;
    return true;
}

bool TCPSocket::bind(int port)
{
    if (connected_ || listening_)
    {
        close();
        initializeSocket();
    }

    // Set socket options for reuse address
    int opt = 1;
    setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (::bind(socket_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        return false;
    }

    return true;
}

bool TCPSocket::listen(int backlog)
{
    if (socket_ == INVALID_SOCKET)
    {
        return false;
    }

    if (::listen(socket_, backlog) == SOCKET_ERROR)
    {
        return false;
    }

    listening_ = true;
    return true;
}

TCPSocket* TCPSocket::accept()
{
    if (!listening_)
    {
        return nullptr;
    }

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    SOCKET clientSocket = ::accept(socket_, (struct sockaddr*)&clientAddr, &clientAddrLen);

    if (clientSocket == INVALID_SOCKET)
    {
        return nullptr;
    }

    // Create a new TCPSocket instance for the accepted connection
    TCPSocket* clientSocketObj = new TCPSocket();
    clientSocketObj->setSocket(clientSocket);

    return clientSocketObj;
}

bool TCPSocket::waitForData(int timeoutMs)
{
    if (!connected_ && !listening_)
    {
        return false;
    }

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(socket_, &readfds);

    struct timeval timeout;
    struct timeval* timeoutPtr = nullptr;

    if (timeoutMs >= 0)
    {
        timeout.tv_sec = timeoutMs / 1000;
        timeout.tv_usec = (timeoutMs % 1000) * 1000;
        timeoutPtr = &timeout;
    }

#ifdef _WIN32
    int selectResult = select(0, &readfds, nullptr, nullptr, timeoutPtr);
#else
    int selectResult = select(socket_ + 1, &readfds, nullptr, nullptr, timeoutPtr);
#endif

    return (selectResult > 0 && FD_ISSET(socket_, &readfds));
}

void TCPSocket::close()
{
    cleanupSocket();
}

bool TCPSocket::isConnected() const
{
    return connected_;
}

SOCKET TCPSocket::getNativeSocket() const
{
    return socket_;
}

} // namespace network_lib

