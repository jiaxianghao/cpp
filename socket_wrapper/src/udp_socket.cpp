#include "network_lib/udp_socket.h"
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

UDPSocket::UDPSocket()
    : socket_(INVALID_SOCKET), connected_(false), bound_(false)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    initializeSocket();
    serverAddrLen_ = sizeof(serverAddr_);
}

UDPSocket::~UDPSocket()
{
    close();
#ifdef _WIN32
    WSACleanup();
#endif
}

void UDPSocket::initializeSocket()
{
    socket_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ == INVALID_SOCKET)
    {
        throw std::runtime_error("Failed to create UDP socket");
    }
}

void UDPSocket::cleanupSocket()
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
    bound_ = false;
}

bool UDPSocket::connect(const std::string& host, int port)
{
    if (connected_)
    {
        close();
        initializeSocket();
    }

    std::memset(&serverAddr_, 0, sizeof(serverAddr_));
    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_port = htons(port);

    // Convert hostname to IP address
    if (inet_pton(AF_INET, host.c_str(), &serverAddr_.sin_addr) <= 0)
    {
        // Try to resolve hostname
        struct hostent* hostEntry = gethostbyname(host.c_str());
        if (hostEntry == nullptr)
        {
            return false;
        }
        std::memcpy(&serverAddr_.sin_addr, hostEntry->h_addr_list[0], hostEntry->h_length);
    }

    // Actually call connect() for UDP (sets default destination)
    if (::connect(socket_, (struct sockaddr*)&serverAddr_, sizeof(serverAddr_)) == SOCKET_ERROR)
    {
        return false;
    }

    connected_ = true;
    return true;
}

bool UDPSocket::send(const void* data, size_t len)
{
    if (!connected_)
    {
        return false;
    }

    // After connect(), can use send() instead of sendto()
    int sent = ::send(socket_, static_cast<const char*>(data), len, 0);

    if (sent == SOCKET_ERROR)
    {
        return false;
    }

    return sent == static_cast<int>(len);
}

bool UDPSocket::receive(void* buffer, size_t& len)
{
    if (!connected_)
    {
        return false;
    }

    // After connect(), can use recv() instead of recvfrom()
    size_t bufferSize = len;
    int received = ::recv(socket_, static_cast<char*>(buffer), bufferSize, 0);

    if (received == SOCKET_ERROR)
    {
        return false;
    }

    len = received;
    return true;
}

bool UDPSocket::bind(int port)
{
    if (connected_ || bound_)
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

    bound_ = true;
    return true;
}

bool UDPSocket::sendTo(const void* data, size_t len, const std::string& host, int port)
{
    struct sockaddr_in destAddr;
    std::memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(port);

    // Convert hostname to IP address
    if (inet_pton(AF_INET, host.c_str(), &destAddr.sin_addr) <= 0)
    {
        struct hostent* hostEntry = gethostbyname(host.c_str());
        if (hostEntry == nullptr)
        {
            return false;
        }
        std::memcpy(&destAddr.sin_addr, hostEntry->h_addr_list[0], hostEntry->h_length);
    }

    int sent = ::sendto(socket_, static_cast<const char*>(data), len, 0,
                        (struct sockaddr*)&destAddr, sizeof(destAddr));

    if (sent == SOCKET_ERROR)
    {
        return false;
    }

    return sent == static_cast<int>(len);
}

bool UDPSocket::receiveFrom(void* buffer, size_t& len, std::string& fromHost, int& fromPort)
{
    struct sockaddr_in fromAddr;
    socklen_t fromAddrLen = sizeof(fromAddr);

    size_t bufferSize = len;
    int received = ::recvfrom(socket_, static_cast<char*>(buffer), bufferSize, 0,
                              (struct sockaddr*)&fromAddr, &fromAddrLen);

    if (received == SOCKET_ERROR)
    {
        return false;
    }

    // Extract source address
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &fromAddr.sin_addr, ipStr, INET_ADDRSTRLEN);
    fromHost = ipStr;
    fromPort = ntohs(fromAddr.sin_port);

    len = received;
    return true;
}

bool UDPSocket::waitForData(int timeoutMs)
{
    if (!connected_ && !bound_)
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

void UDPSocket::close()
{
    cleanupSocket();
}

bool UDPSocket::isConnected() const
{
    return connected_;
}

SOCKET UDPSocket::getNativeSocket() const
{
    return socket_;
}

} // namespace network_lib

