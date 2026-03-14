# Transport Layer Abstraction Library

A unified C++ interface for multiple communication protocols on Linux, including CAN, Serial Port, Socket, and Shared Memory.

## Features

- **Unified Interface**: All transport types implement the same `ITransport` interface
- **Thread-Safe**: All operations are protected by mutexes
- **RAII**: Automatic resource management and cleanup
- **Factory Pattern**: Easy creation of transport instances
- **Linux Native**: Uses Linux APIs for optimal performance

## Supported Transport Types

### 1. Serial Port Communication
- Based on POSIX termios API
- Supports various baud rates, data bits, stop bits, and parity
- Device paths: `/dev/ttyS0`, `/dev/ttyUSB0`, etc.

### 2. Socket Communication
- TCP and UDP support
- IPv4 and IPv6 support
- Client and server modes
- Standard Linux socket API

### 3. Shared Memory Communication
- POSIX shared memory (shm_open, mmap)
- Synchronized with POSIX semaphores
- Named shared memory segments

### 4. CAN Communication
- Linux SocketCAN support
- Standard CAN and CAN FD
- RAW socket interface

## Building

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake

# Fedora/RHEL
sudo dnf install gcc-c++ cmake
```

### Build Instructions

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
make

# Install (optional)
sudo make install
```

### Build Options

- `BUILD_EXAMPLES`: Build example programs (default: ON)

```bash
cmake -DBUILD_EXAMPLES=OFF ..
```

## Usage

### Basic Example

```cpp
#include "TransportFactory.h"
#include <iostream>

using namespace transport;

int main()
{
    // Create a serial transport
    SerialConfig config;
    config.device = "/dev/ttyUSB0";
    config.baudrate = 115200;

    auto transport = TransportFactory::CreateSerial(config);

    // Open the connection
    if (!transport->Open())
    {
        std::cerr << "Failed to open: " << transport->GetLastError() << std::endl;
        return 1;
    }

    // Send data
    const char* message = "Hello!";
    int sent = transport->Send(message, strlen(message));

    // Receive data
    char buffer[256];
    int received = transport->Receive(buffer, sizeof(buffer));

    // Close automatically when transport goes out of scope
    return 0;
}
```

### Serial Port Example

```cpp
SerialConfig config;
config.device = "/dev/ttyUSB0";
config.baudrate = 115200;
config.databits = 8;
config.stopbits = 1;
config.parity = 'N';

auto transport = TransportFactory::CreateSerial(config);
```

### TCP Socket Client Example

```cpp
SocketConfig config;
config.address = "192.168.1.100";
config.port = 8080;
config.protocol = SocketConfig::Protocol::TCP;
config.is_server = false;

auto transport = TransportFactory::CreateSocket(config);
```

### TCP Socket Server Example

```cpp
SocketConfig config;
config.address = "0.0.0.0";
config.port = 8080;
config.protocol = SocketConfig::Protocol::TCP;
config.is_server = true;

auto transport = TransportFactory::CreateSocket(config);
// Open() will wait for a client connection
```

### Shared Memory Example

```cpp
// Writer
SharedMemoryConfig config;
config.name = "/my_shm";
config.size = 4096;
config.create = true;

auto transport = TransportFactory::CreateSharedMemory(config);

// Reader (in another process)
SharedMemoryConfig config;
config.name = "/my_shm";
config.size = 4096;
config.create = false;

auto transport = TransportFactory::CreateSharedMemory(config);
```

### CAN Example

```cpp
CanConfig config;
config.interface = "can0";
config.use_canfd = false;

auto transport = TransportFactory::CreateCan(config);

// Send CAN frame
struct can_frame frame;
frame.can_id = 0x123;
frame.can_dlc = 8;
// ... set data ...
transport->Send(&frame, sizeof(frame));
```

## Setting up Virtual CAN for Testing

```bash
# Load vcan module
sudo modprobe vcan

# Create virtual CAN interface
sudo ip link add dev vcan0 type vcan

# Bring up the interface
sudo ip link set up vcan0

# Verify
ip link show vcan0
```

## Permissions

### Serial Port

```bash
# Add user to dialout group
sudo usermod -a -G dialout $USER

# Or change device permissions
sudo chmod 666 /dev/ttyUSB0
```

### Shared Memory

Shared memory objects are created in `/dev/shm/`. Ensure proper permissions:

```bash
# Check permissions
ls -la /dev/shm/

# Clean up shared memory
rm /dev/shm/my_shm
```

### CAN Interface

```bash
# Give user permission to use CAN
sudo ip link set can0 up type can bitrate 500000
```

## API Reference

### ITransport Interface

```cpp
class ITransport
{
public:
    virtual bool Open() = 0;
    virtual void Close() = 0;
    virtual int Send(const void* data, size_t size) = 0;
    virtual int Receive(void* buffer, size_t size) = 0;
    virtual bool IsConnected() const = 0;
    virtual std::string GetLastError() const = 0;
    virtual TransportType GetType() const = 0;
};
```

- `Open()`: Open the transport connection. Returns true on success.
- `Close()`: Close the transport connection.
- `Send()`: Send data. Returns number of bytes sent, or -1 on error.
- `Receive()`: Receive data. Returns number of bytes received, or -1 on error.
- `IsConnected()`: Check if the transport is connected.
- `GetLastError()`: Get the last error message.
- `GetType()`: Get the transport type.

## Project Structure

```
io/
├── include/
│   ├── ITransport.h                # Core interface
│   ├── TransportBase.h             # Common base class
│   ├── CanTransport.h              # CAN implementation
│   ├── SerialTransport.h           # Serial port implementation
│   ├── SharedMemoryTransport.h     # Shared memory implementation
│   ├── SocketTransport.h           # Socket implementation
│   └── TransportFactory.h          # Factory class
├── src/
│   ├── TransportBase.cpp
│   ├── CanTransport.cpp
│   ├── SerialTransport.cpp
│   ├── SharedMemoryTransport.cpp
│   ├── SocketTransport.cpp
│   └── TransportFactory.cpp
├── examples/
│   └── main.cpp                    # Usage examples
├── CMakeLists.txt                  # Build configuration
└── README.md                       # This file
```

## Design Principles

1. **Interface Abstraction**: All transport types implement the same interface
2. **Thread Safety**: Mutex protection for all operations
3. **RAII**: Automatic resource cleanup in destructors
4. **Error Handling**: Consistent error reporting across all types
5. **Linux Native**: Uses native Linux APIs for best performance

## License

This is a sample implementation for educational purposes.

## Contributing

Feel free to extend this library with additional transport types or features!
