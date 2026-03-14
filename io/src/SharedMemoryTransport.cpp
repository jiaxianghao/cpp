#include "SharedMemoryTransport.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

namespace transport
{

SharedMemoryTransport::SharedMemoryTransport(const SharedMemoryConfig& config)
    : m_config(config)
    , m_shmAddr(MAP_FAILED)
    , m_writeSem(SEM_FAILED)
    , m_readSem(SEM_FAILED)
    , m_creator(false)
{
}

SharedMemoryTransport::~SharedMemoryTransport()
{
    Close();
}

bool SharedMemoryTransport::Open()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_connected)
    {
        SetError("Shared memory already open");
        return false;
    }

    int flags = O_RDWR;
    if (m_config.create)
    {
        flags |= O_CREAT;
        m_creator = true;
    }

    // Open or create shared memory
    m_fd = shm_open(m_config.name.c_str(), flags, 0666);
    if (m_fd < 0)
    {
        SetErrorFromErrno("Failed to open shared memory");
        return false;
    }

    // Set size if creating
    if (m_config.create)
    {
        size_t total_size = sizeof(ShmHeader) + m_config.size;
        if (ftruncate(m_fd, total_size) < 0)
        {
            SetErrorFromErrno("Failed to set shared memory size");
            ::close(m_fd);
            m_fd = -1;
            if (m_creator)
            {
                shm_unlink(m_config.name.c_str());
            }
            return false;
        }
    }

    // Map shared memory
    size_t total_size = sizeof(ShmHeader) + m_config.size;
    m_shmAddr = mmap(nullptr, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
    if (m_shmAddr == MAP_FAILED)
    {
        SetErrorFromErrno("Failed to map shared memory");
        ::close(m_fd);
        m_fd = -1;
        if (m_creator)
        {
            shm_unlink(m_config.name.c_str());
        }
        return false;
    }

    // Initialize header if creating
    if (m_config.create)
    {
        ShmHeader* header = static_cast<ShmHeader*>(m_shmAddr);
        header->data_size = 0;
        header->has_data = false;
    }

    // Open or create semaphores
    std::string write_sem_name = m_config.name + "_write";
    std::string read_sem_name = m_config.name + "_read";

    if (m_config.create)
    {
        // Create semaphores
        m_writeSem = sem_open(write_sem_name.c_str(), O_CREAT, 0666, 1);
        m_readSem = sem_open(read_sem_name.c_str(), O_CREAT, 0666, 0);
    }
    else
    {
        // Open existing semaphores
        m_writeSem = sem_open(write_sem_name.c_str(), 0);
        m_readSem = sem_open(read_sem_name.c_str(), 0);
    }

    if (m_writeSem == SEM_FAILED || m_readSem == SEM_FAILED)
    {
        SetErrorFromErrno("Failed to open semaphores");
        if (m_writeSem != SEM_FAILED)
        {
            sem_close(m_writeSem);
            m_writeSem = SEM_FAILED;
        }
        if (m_readSem != SEM_FAILED)
        {
            sem_close(m_readSem);
            m_readSem = SEM_FAILED;
        }
        munmap(m_shmAddr, total_size);
        m_shmAddr = MAP_FAILED;
        ::close(m_fd);
        m_fd = -1;
        if (m_creator)
        {
            shm_unlink(m_config.name.c_str());
            sem_unlink(write_sem_name.c_str());
            sem_unlink(read_sem_name.c_str());
        }
        return false;
    }

    SetConnected(true);
    return true;
}

void SharedMemoryTransport::Close()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // Close semaphores
    if (m_writeSem != SEM_FAILED)
    {
        sem_close(m_writeSem);
        if (m_creator)
        {
            std::string write_sem_name = m_config.name + "_write";
            sem_unlink(write_sem_name.c_str());
        }
        m_writeSem = SEM_FAILED;
    }

    if (m_readSem != SEM_FAILED)
    {
        sem_close(m_readSem);
        if (m_creator)
        {
            std::string read_sem_name = m_config.name + "_read";
            sem_unlink(read_sem_name.c_str());
        }
        m_readSem = SEM_FAILED;
    }

    // Unmap shared memory
    if (m_shmAddr != MAP_FAILED)
    {
        size_t total_size = sizeof(ShmHeader) + m_config.size;
        munmap(m_shmAddr, total_size);
        m_shmAddr = MAP_FAILED;
    }

    // Close and unlink shared memory
    if (m_fd >= 0)
    {
        ::close(m_fd);
        if (m_creator)
        {
            shm_unlink(m_config.name.c_str());
        }
        m_fd = -1;
    }

    SetConnected(false);
}

int SharedMemoryTransport::Send(const void* data, size_t size)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_connected || m_shmAddr == MAP_FAILED)
    {
        SetError("Shared memory not connected");
        return -1;
    }

    if (size > m_config.size)
    {
        SetError("Data size exceeds shared memory buffer size");
        return -1;
    }

    // Wait for write permission
    if (sem_wait(m_writeSem) < 0)
    {
        SetErrorFromErrno("Failed to wait on write semaphore");
        return -1;
    }

    // Write data to shared memory
    ShmHeader* header = static_cast<ShmHeader*>(m_shmAddr);
    void* data_ptr = static_cast<char*>(m_shmAddr) + sizeof(ShmHeader);

    std::memcpy(data_ptr, data, size);
    header->data_size = size;
    header->has_data = true;

    // Signal that data is available
    if (sem_post(m_readSem) < 0)
    {
        SetErrorFromErrno("Failed to post read semaphore");
        sem_post(m_writeSem);  // Release write lock
        return -1;
    }

    return static_cast<int>(size);
}

int SharedMemoryTransport::Receive(void* buffer, size_t size)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_connected || m_shmAddr == MAP_FAILED)
    {
        SetError("Shared memory not connected");
        return -1;
    }

    // Wait for data to be available
    if (sem_wait(m_readSem) < 0)
    {
        SetErrorFromErrno("Failed to wait on read semaphore");
        return -1;
    }

    // Read data from shared memory
    ShmHeader* header = static_cast<ShmHeader*>(m_shmAddr);
    void* data_ptr = static_cast<char*>(m_shmAddr) + sizeof(ShmHeader);

    if (!header->has_data)
    {
        SetError("No data available in shared memory");
        sem_post(m_writeSem);  // Release write lock
        return -1;
    }

    size_t bytes_to_read = (header->data_size < size) ? header->data_size : size;
    std::memcpy(buffer, data_ptr, bytes_to_read);

    header->has_data = false;
    header->data_size = 0;

    // Signal that write can proceed
    if (sem_post(m_writeSem) < 0)
    {
        SetErrorFromErrno("Failed to post write semaphore");
        return -1;
    }

    return static_cast<int>(bytes_to_read);
}

TransportType SharedMemoryTransport::GetType() const
{
    return TransportType::SHARED_MEMORY;
}

} // namespace transport
