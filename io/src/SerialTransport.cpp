#include "SerialTransport.h"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

namespace transport
{

SerialTransport::SerialTransport(const SerialConfig& config)
    : m_config(config)
{
}

SerialTransport::~SerialTransport()
{
    Close();
}

bool SerialTransport::Open()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_connected)
    {
        SetError("Serial port already open");
        return false;
    }

    // Open the serial port device
    m_fd = ::open(m_config.device.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (m_fd < 0)
    {
        SetErrorFromErrno("Failed to open serial device");
        return false;
    }

    // Configure the serial port
    if (!ConfigurePort())
    {
        ::close(m_fd);
        m_fd = -1;
        return false;
    }

    SetConnected(true);
    return true;
}

void SerialTransport::Close()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_fd >= 0)
    {
        ::close(m_fd);
        m_fd = -1;
    }

    SetConnected(false);
}

int SerialTransport::Send(const void* data, size_t size)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_connected || m_fd < 0)
    {
        SetError("Serial port not connected");
        return -1;
    }

    ssize_t result = ::write(m_fd, data, size);
    if (result < 0)
    {
        SetErrorFromErrno("Failed to write to serial port");
        return -1;
    }

    return static_cast<int>(result);
}

int SerialTransport::Receive(void* buffer, size_t size)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_connected || m_fd < 0)
    {
        SetError("Serial port not connected");
        return -1;
    }

    ssize_t result = ::read(m_fd, buffer, size);
    if (result < 0)
    {
        SetErrorFromErrno("Failed to read from serial port");
        return -1;
    }

    return static_cast<int>(result);
}

TransportType SerialTransport::GetType() const
{
    return TransportType::SERIAL;
}

bool SerialTransport::ConfigurePort()
{
    struct termios tty;

    // Get current attributes
    if (tcgetattr(m_fd, &tty) != 0)
    {
        SetErrorFromErrno("Failed to get serial port attributes");
        return false;
    }

    // Set baud rate
    int baud = GetBaudRateConstant(m_config.baudrate);
    if (baud == -1)
    {
        SetError("Invalid baud rate");
        return false;
    }
    cfsetospeed(&tty, baud);
    cfsetispeed(&tty, baud);

    // Set data bits
    tty.c_cflag &= ~CSIZE;
    switch (m_config.databits)
    {
        case 5: tty.c_cflag |= CS5; break;
        case 6: tty.c_cflag |= CS6; break;
        case 7: tty.c_cflag |= CS7; break;
        case 8: tty.c_cflag |= CS8; break;
        default:
            SetError("Invalid data bits");
            return false;
    }

    // Set parity
    switch (m_config.parity)
    {
        case 'N':
        case 'n':
            tty.c_cflag &= ~PARENB;
            break;
        case 'E':
        case 'e':
            tty.c_cflag |= PARENB;
            tty.c_cflag &= ~PARODD;
            break;
        case 'O':
        case 'o':
            tty.c_cflag |= PARENB;
            tty.c_cflag |= PARODD;
            break;
        default:
            SetError("Invalid parity");
            return false;
    }

    // Set stop bits
    if (m_config.stopbits == 1)
    {
        tty.c_cflag &= ~CSTOPB;
    }
    else if (m_config.stopbits == 2)
    {
        tty.c_cflag |= CSTOPB;
    }
    else
    {
        SetError("Invalid stop bits");
        return false;
    }

    // Control modes
    tty.c_cflag |= (CLOCAL | CREAD);  // Enable receiver, ignore modem control lines

    // Input modes
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);  // Disable software flow control
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    // Output modes
    tty.c_oflag &= ~OPOST;  // Raw output

    // Local modes
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

    // Read timeout and minimum bytes
    tty.c_cc[VMIN] = 0;   // Non-blocking read
    tty.c_cc[VTIME] = 1;  // 0.1 second timeout

    // Apply settings
    if (tcsetattr(m_fd, TCSANOW, &tty) != 0)
    {
        SetErrorFromErrno("Failed to set serial port attributes");
        return false;
    }

    return true;
}

int SerialTransport::GetBaudRateConstant(int baudrate)
{
    switch (baudrate)
    {
        case 50: return B50;
        case 75: return B75;
        case 110: return B110;
        case 134: return B134;
        case 150: return B150;
        case 200: return B200;
        case 300: return B300;
        case 600: return B600;
        case 1200: return B1200;
        case 1800: return B1800;
        case 2400: return B2400;
        case 4800: return B4800;
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        case 460800: return B460800;
        case 500000: return B500000;
        case 576000: return B576000;
        case 921600: return B921600;
        case 1000000: return B1000000;
        case 1152000: return B1152000;
        case 1500000: return B1500000;
        case 2000000: return B2000000;
        case 2500000: return B2500000;
        case 3000000: return B3000000;
        case 3500000: return B3500000;
        case 4000000: return B4000000;
        default: return -1;
    }
}

} // namespace transport
