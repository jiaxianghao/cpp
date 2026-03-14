#ifndef SHARED_MEMORY_TRANSPORT_H
#define SHARED_MEMORY_TRANSPORT_H

#include "TransportBase.h"
#include <semaphore.h>

namespace transport
{

// Shared memory transport implementation
class SharedMemoryTransport : public TransportBase
{
public:
    explicit SharedMemoryTransport(const SharedMemoryConfig& config);
    virtual ~SharedMemoryTransport() override;

    // ITransport interface implementation
    bool Open() override;
    void Close() override;
    int Send(const void* data, size_t size) override;
    int Receive(void* buffer, size_t size) override;
    TransportType GetType() const override;

private:
    // Shared memory header structure
    struct ShmHeader
    {
        size_t data_size;    // Size of data in buffer
        bool has_data;       // Flag indicating if data is available
    };

    SharedMemoryConfig m_config;
    void* m_shmAddr;         // Shared memory address
    sem_t* m_writeSem;       // Semaphore for write synchronization
    sem_t* m_readSem;        // Semaphore for read synchronization
    bool m_creator;          // true if this instance created the shared memory
};

} // namespace transport

#endif // SHARED_MEMORY_TRANSPORT_H
