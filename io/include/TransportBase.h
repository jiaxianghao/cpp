#ifndef TRANSPORT_BASE_H
#define TRANSPORT_BASE_H

#include "ITransport.h"
#include <mutex>
#include <string>
#include <cstring>
#include <cerrno>

namespace transport
{

// Base class implementing common functionality for all transports
class TransportBase : public ITransport
{
public:
    TransportBase();
    virtual ~TransportBase() override;

    // ITransport interface implementation
    bool IsConnected() const override;
    std::string GetLastError() const override;

protected:
    // Set connection state
    void SetConnected(bool connected);

    // Set error message from string
    void SetError(const std::string& error);

    // Set error message from errno
    void SetErrorFromErrno(const std::string& context);

    // Thread safety
    mutable std::mutex m_mutex;

    // Connection state
    bool m_connected;

    // Last error message
    std::string m_lastError;

    // File descriptor (used by most Linux transport implementations)
    int m_fd;
};

} // namespace transport

#endif // TRANSPORT_BASE_H
