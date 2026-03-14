#ifndef NETWORK_LIB_UDP_SOCKET_H
#define NETWORK_LIB_UDP_SOCKET_H

#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

namespace network_lib
{

// UDP socket wrapper providing convenient functions
class UDPSocket
{
public:
    UDPSocket();
    ~UDPSocket();

    // Client operations (UDP connect mode)
    // Connect sets default destination for send/receive
    bool connect(const std::string& host, int port);

    // Send data (requires connect() first or use sendTo)
    bool send(const void* data, size_t len);

    // Receive data (requires connect() first or use receiveFrom)
    bool receive(void* buffer, size_t& len);

    // Server operations
    // Bind to port
    bool bind(int port);

    // Send to specific address
    bool sendTo(const void* data, size_t len, const std::string& host, int port);

    // Receive from any address
    bool receiveFrom(void* buffer, size_t& len, std::string& fromHost, int& fromPort);

    // Wait for data to be available using select()
    // timeoutMs: timeout in milliseconds, -1 for blocking, 0 for non-blocking
    // Returns true if data is available
    bool waitForData(int timeoutMs = -1);

    // Close socket
    void close();

    // Check if connected (UDP connect mode)
    bool isConnected() const;

    // Get native socket handle
    SOCKET getNativeSocket() const;

private:
    SOCKET socket_;
    bool connected_;
    bool bound_;
    struct sockaddr_in serverAddr_;
    socklen_t serverAddrLen_;

    void initializeSocket();
    void cleanupSocket();
};

} // namespace network_lib

#endif // NETWORK_LIB_UDP_SOCKET_H

