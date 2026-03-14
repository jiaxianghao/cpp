#ifndef ITRANSPORT_H
#define ITRANSPORT_H

#include <string>
#include <cstddef>
#include <memory>

namespace transport
{

// Transport type enumeration
enum class TransportType
{
    SERIAL,
    SOCKET,
    SHARED_MEMORY,
    CAN
};

// Base configuration structure
struct TransportConfig
{
    virtual ~TransportConfig() = default;
};

// Serial port configuration
struct SerialConfig : public TransportConfig
{
    std::string device;      // Device path: /dev/ttyS0, /dev/ttyUSB0, etc.
    int baudrate;            // Baud rate: 9600, 115200, etc.
    int databits;            // Data bits: 5, 6, 7, 8
    int stopbits;            // Stop bits: 1, 2
    char parity;             // Parity: 'N' (none), 'E' (even), 'O' (odd)

    SerialConfig()
        : device("/dev/ttyS0"), baudrate(115200), databits(8), stopbits(1), parity('N')
    {
    }
};

// Socket configuration
struct SocketConfig : public TransportConfig
{
    enum class Protocol
    {
        TCP,
        UDP
    };

    std::string address;     // IP address or hostname
    int port;                // Port number
    Protocol protocol;       // TCP or UDP
    bool is_server;          // true for server mode, false for client mode

    SocketConfig()
        : address("127.0.0.1"), port(8080), protocol(Protocol::TCP), is_server(false)
    {
    }
};

// Shared memory configuration
struct SharedMemoryConfig : public TransportConfig
{
    std::string name;        // Shared memory name
    size_t size;             // Size of shared memory in bytes
    bool create;             // true to create, false to attach existing

    SharedMemoryConfig()
        : name("/shm_transport"), size(4096), create(true)
    {
    }
};

// CAN configuration
struct CanConfig : public TransportConfig
{
    std::string interface;   // CAN interface name: can0, vcan0, etc.
    bool use_canfd;          // true for CAN FD, false for standard CAN

    CanConfig()
        : interface("can0"), use_canfd(false)
    {
    }
};

// Transport interface
class ITransport
{
public:
    virtual ~ITransport() = default;

    // Open the transport connection
    // Returns true on success, false on failure
    virtual bool Open() = 0;

    // Close the transport connection
    virtual void Close() = 0;

    // Send data through the transport
    // Returns the number of bytes sent, or -1 on error
    virtual int Send(const void* data, size_t size) = 0;

    // Receive data from the transport
    // Returns the number of bytes received, or -1 on error
    virtual int Receive(void* buffer, size_t size) = 0;

    // Check if the transport is connected
    virtual bool IsConnected() const = 0;

    // Get the last error message
    virtual std::string GetLastError() const = 0;

    // Get transport type
    virtual TransportType GetType() const = 0;
};

// Type alias for smart pointer
using TransportPtr = std::unique_ptr<ITransport>;

} // namespace transport

#endif // ITRANSPORT_H
