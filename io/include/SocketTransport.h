#ifndef SOCKET_TRANSPORT_H
#define SOCKET_TRANSPORT_H

#include "TransportBase.h"
#include <netinet/in.h>

namespace transport
{

// Socket transport implementation
class SocketTransport : public TransportBase
{
public:
    explicit SocketTransport(const SocketConfig& config);
    virtual ~SocketTransport() override;

    // ITransport interface implementation
    bool Open() override;
    void Close() override;
    int Send(const void* data, size_t size) override;
    int Receive(void* buffer, size_t size) override;
    TransportType GetType() const override;

private:
    // Setup TCP connection
    bool SetupTCP();

    // Setup UDP connection
    bool SetupUDP();

    // Resolve hostname to IP address
    bool ResolveAddress(const std::string& address, struct sockaddr_in& addr);

    SocketConfig m_config;
    int m_clientFd;  // For server mode: accepted client socket
};

} // namespace transport

#endif // SOCKET_TRANSPORT_H
