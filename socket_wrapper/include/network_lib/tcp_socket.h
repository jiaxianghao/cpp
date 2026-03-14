#ifndef NETWORK_LIB_TCP_SOCKET_H
#define NETWORK_LIB_TCP_SOCKET_H

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

// TCP socket wrapper providing convenient functions
class TCPSocket
{
public:
    TCPSocket();
    ~TCPSocket();

    // Client operations
    // Connect to remote host
    bool connect(const std::string& host, int port);

    // Send data
    bool send(const void* data, size_t len);

    // Receive data
    // len: input as buffer size, output as actual received bytes
    bool receive(void* buffer, size_t& len);

    // Server operations
    // Bind to port
    bool bind(int port);

    // Start listening
    bool listen(int backlog = 5);

    // Accept incoming connection
    // Returns new TCPSocket for accepted connection, nullptr if failed
    TCPSocket* accept();

    // Wait for data to be available using select()
    // timeoutMs: timeout in milliseconds, -1 for blocking, 0 for non-blocking
    // Returns true if data is available
    bool waitForData(int timeoutMs = -1);

    // Close socket
    void close();

    // Check if connected
    bool isConnected() const;

    // Get native socket handle
    SOCKET getNativeSocket() const;

private:
    SOCKET socket_;
    bool connected_;
    bool listening_;

    void initializeSocket();
    void cleanupSocket();
    void setSocket(SOCKET socket);
};

} // namespace network_lib

#endif // NETWORK_LIB_TCP_SOCKET_H

