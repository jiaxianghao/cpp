#include "TransportFactory.h"

namespace transport
{

TransportPtr TransportFactory::CreateSerial(const SerialConfig& config)
{
    return std::make_unique<SerialTransport>(config);
}

TransportPtr TransportFactory::CreateSocket(const SocketConfig& config)
{
    return std::make_unique<SocketTransport>(config);
}

TransportPtr TransportFactory::CreateSharedMemory(const SharedMemoryConfig& config)
{
    return std::make_unique<SharedMemoryTransport>(config);
}

TransportPtr TransportFactory::CreateCan(const CanConfig& config)
{
    return std::make_unique<CanTransport>(config);
}

TransportPtr TransportFactory::Create(TransportType type, const TransportConfig& config)
{
    switch (type)
    {
        case TransportType::SERIAL:
            return CreateSerial(static_cast<const SerialConfig&>(config));

        case TransportType::SOCKET:
            return CreateSocket(static_cast<const SocketConfig&>(config));

        case TransportType::SHARED_MEMORY:
            return CreateSharedMemory(static_cast<const SharedMemoryConfig&>(config));

        case TransportType::CAN:
            return CreateCan(static_cast<const CanConfig&>(config));

        default:
            return nullptr;
    }
}

} // namespace transport
