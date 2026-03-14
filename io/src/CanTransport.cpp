#include "CanTransport.h"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <unistd.h>
#include <cstring>

namespace transport
{

CanTransport::CanTransport(const CanConfig& config)
    : m_config(config)
{
}

CanTransport::~CanTransport()
{
    Close();
}

bool CanTransport::Open()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_connected)
    {
        SetError("CAN interface already open");
        return false;
    }

    // Create CAN socket
    int socket_type = SOCK_RAW;
    int protocol = CAN_RAW;

    m_fd = ::socket(PF_CAN, socket_type, protocol);
    if (m_fd < 0)
    {
        SetErrorFromErrno("Failed to create CAN socket");
        return false;
    }

    // Get interface index
    int ifindex = GetInterfaceIndex(m_config.interface);
    if (ifindex < 0)
    {
        ::close(m_fd);
        m_fd = -1;
        return false;
    }

    // Bind socket to CAN interface
    struct sockaddr_can addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifindex;

    if (::bind(m_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        SetErrorFromErrno("Failed to bind CAN socket");
        ::close(m_fd);
        m_fd = -1;
        return false;
    }

    // Enable CAN FD if requested
    if (m_config.use_canfd)
    {
        int enable = 1;
        if (setsockopt(m_fd, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable, sizeof(enable)) < 0)
        {
            SetErrorFromErrno("Failed to enable CAN FD");
            ::close(m_fd);
            m_fd = -1;
            return false;
        }
    }

    SetConnected(true);
    return true;
}

void CanTransport::Close()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_fd >= 0)
    {
        ::close(m_fd);
        m_fd = -1;
    }

    SetConnected(false);
}

int CanTransport::Send(const void* data, size_t size)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_connected || m_fd < 0)
    {
        SetError("CAN interface not connected");
        return -1;
    }

    // Validate size
    if (m_config.use_canfd)
    {
        if (size > sizeof(struct canfd_frame))
        {
            SetError("Data size exceeds CAN FD frame size");
            return -1;
        }
    }
    else
    {
        if (size > sizeof(struct can_frame))
        {
            SetError("Data size exceeds CAN frame size");
            return -1;
        }
    }

    ssize_t result = ::write(m_fd, data, size);
    if (result < 0)
    {
        SetErrorFromErrno("Failed to send CAN frame");
        return -1;
    }

    return static_cast<int>(result);
}

int CanTransport::Receive(void* buffer, size_t size)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_connected || m_fd < 0)
    {
        SetError("CAN interface not connected");
        return -1;
    }

    // Validate size
    if (m_config.use_canfd)
    {
        if (size < sizeof(struct canfd_frame))
        {
            SetError("Buffer size too small for CAN FD frame");
            return -1;
        }
    }
    else
    {
        if (size < sizeof(struct can_frame))
        {
            SetError("Buffer size too small for CAN frame");
            return -1;
        }
    }

    ssize_t result = ::read(m_fd, buffer, size);
    if (result < 0)
    {
        SetErrorFromErrno("Failed to receive CAN frame");
        return -1;
    }

    return static_cast<int>(result);
}

TransportType CanTransport::GetType() const
{
    return TransportType::CAN;
}

int CanTransport::GetInterfaceIndex(const std::string& interface)
{
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);

    if (ioctl(m_fd, SIOCGIFINDEX, &ifr) < 0)
    {
        SetErrorFromErrno("Failed to get CAN interface index");
        return -1;
    }

    return ifr.ifr_ifindex;
}

} // namespace transport
