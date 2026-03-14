#ifndef TRANSPORT_FACTORY_H
#define TRANSPORT_FACTORY_H

#include "ITransport.h"
#include "SerialTransport.h"
#include "SocketTransport.h"
#include "SharedMemoryTransport.h"
#include "CanTransport.h"

namespace transport
{

// Factory class for creating transport instances
class TransportFactory
{
public:
    // Create a serial transport instance
    static TransportPtr CreateSerial(const SerialConfig& config);

    // Create a socket transport instance
    static TransportPtr CreateSocket(const SocketConfig& config);

    // Create a shared memory transport instance
    static TransportPtr CreateSharedMemory(const SharedMemoryConfig& config);

    // Create a CAN transport instance
    static TransportPtr CreateCan(const CanConfig& config);

    // Create a transport instance based on type and generic config
    static TransportPtr Create(TransportType type, const TransportConfig& config);

private:
    // Private constructor to prevent instantiation
    TransportFactory() = delete;
};

} // namespace transport

#endif // TRANSPORT_FACTORY_H
