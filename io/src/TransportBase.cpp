#include "TransportBase.h"
#include <unistd.h>

namespace transport
{

TransportBase::TransportBase()
    : m_connected(false)
    , m_lastError("")
    , m_fd(-1)
{
}

TransportBase::~TransportBase()
{
    if (m_fd >= 0)
    {
        ::close(m_fd);
        m_fd = -1;
    }
}

bool TransportBase::IsConnected() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_connected;
}

std::string TransportBase::GetLastError() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_lastError;
}

void TransportBase::SetConnected(bool connected)
{
    m_connected = connected;
}

void TransportBase::SetError(const std::string& error)
{
    m_lastError = error;
}

void TransportBase::SetErrorFromErrno(const std::string& context)
{
    char buf[256];
    strerror_r(errno, buf, sizeof(buf));
    m_lastError = context + ": " + std::string(buf);
}

} // namespace transport
