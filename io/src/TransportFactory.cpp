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
        {
            auto serial_config = dynamic_cast<const SerialConfig*>(&config);
            return serial_config ? CreateSerial(*serial_config) : nullptr;
        }

        case TransportType::SOCKET:
        {
            auto socket_config = dynamic_cast<const SocketConfig*>(&config);
            return socket_config ? CreateSocket(*socket_config) : nullptr;
        }

        case TransportType::SHARED_MEMORY:
        {
            auto shm_config = dynamic_cast<const SharedMemoryConfig*>(&config);
            return shm_config ? CreateSharedMemory(*shm_config) : nullptr;
        }

        case TransportType::CAN:
        {
            auto can_config = dynamic_cast<const CanConfig*>(&config);
            return can_config ? CreateCan(*can_config) : nullptr;
        }

        default:
            return nullptr;
    }
}

} // namespace transport
