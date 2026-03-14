#ifndef CAN_TRANSPORT_H
#define CAN_TRANSPORT_H

#include "TransportBase.h"

namespace transport
{

// CAN transport implementation using SocketCAN
class CanTransport : public TransportBase
{
public:
    explicit CanTransport(const CanConfig& config);
    virtual ~CanTransport() override;

    // ITransport interface implementation
    bool Open() override;
    void Close() override;
    int Send(const void* data, size_t size) override;
    int Receive(void* buffer, size_t size) override;
    TransportType GetType() const override;

private:
    // Get interface index from interface name
    int GetInterfaceIndex(const std::string& interface);

    CanConfig m_config;
};

} // namespace transport

#endif // CAN_TRANSPORT_H
