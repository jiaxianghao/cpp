# Network Communication Library

A C++ network communication library that supports both TCP and UDP protocols with extensible protocol support.

## Features

- **Transport Layer Abstraction**: Support for both TCP and UDP with runtime switching
- **Client and Server**: Complete client and server implementations
- **Protocol Extension**: Pluggable protocol system for custom encoding/decoding
- **Cross-platform**: Works on Linux (primary target) and Windows

## Architecture

### Core Components

1. **Transport Layer** (`transport/`)
   - `Transport`: Base class defining transport interface
   - `TCPTransport`: TCP implementation
   - `UDPTransport`: UDP implementation

2. **Protocol Layer** (`protocol/`)
   - `Protocol`: Base class for protocol implementations
   - Extensible interface for custom protocols

3. **Client** (`client.h/cpp`)
   - Network client supporting TCP/UDP
   - Integrated with protocol layer

4. **Server** (`server.h/cpp`)
   - Network server supporting TCP/UDP
   - Callback-based data handling

## Building

### Requirements

- CMake 3.10 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+)
- Linux system (primary target)

### Build Steps

```bash
mkdir build
cd build
cmake ..
make
```

### Build Examples

```bash
make
./tcp_example
./udp_example
./protocol_example
```

## Usage

### Basic TCP Client

```cpp
#include "network_lib/client.h"
#include "network_lib/tcp_transport.h"
#include "simple_protocol.h"

// Create transport and protocol
network_lib::TCPTransport* transport = new network_lib::TCPTransport();
SimpleProtocol* protocol = new SimpleProtocol();

// Create client
network_lib::Client client(transport, protocol);

// Connect and send
if (client.connect("127.0.0.1", 8888))
{
    std::string message = "Hello, Server!";
    client.send(message.c_str(), message.length());
    
    // Receive response
    auto data = client.receive();
    // Process data...
    
    client.disconnect();
}
```

### Basic TCP Server

```cpp
#include "network_lib/server.h"
#include "network_lib/tcp_transport.h"
#include "simple_protocol.h"

// Create transport and protocol
network_lib::TCPTransport* transport = new network_lib::TCPTransport();
SimpleProtocol* protocol = new SimpleProtocol();

// Create server
network_lib::Server server(transport, protocol);

// Start server
if (server.start(8888))
{
    // Handle connections
    network_lib::DataHandler handler = [](const uint8_t* data, size_t len)
    {
        // Process received data
        std::string message(reinterpret_cast<const char*>(data), len);
        std::cout << "Received: " << message << std::endl;
    };
    
    server.accept(handler);
    
    server.stop();
}
```

### UDP Usage

Replace `TCPTransport` with `UDPTransport`:

```cpp
network_lib::UDPTransport* transport = new network_lib::UDPTransport();
```

### Custom Protocol Implementation

Extend the `Protocol` base class:

```cpp
#include "network_lib/protocol.h"

class MyProtocol : public network_lib::Protocol
{
public:
    std::vector<uint8_t> encode(const void* data, size_t len) override
    {
        // Your encoding logic
        std::vector<uint8_t> result;
        // ... encode data ...
        return result;
    }

    std::vector<uint8_t> decode(const void* data, size_t len) override
    {
        // Your decoding logic
        std::vector<uint8_t> result;
        // ... decode data ...
        return result;
    }
};
```

Then use it with Client or Server:

```cpp
MyProtocol* protocol = new MyProtocol();
network_lib::Client client(transport, protocol);
```

## Examples

The `examples/` directory contains:

- `tcp_client_server.cpp`: TCP client-server example
- `udp_client_server.cpp`: UDP client-server example
- `protocol_example.cpp`: Protocol extension example
- `simple_protocol.h/cpp`: Simple pass-through protocol implementation

## Design Principles

1. **Separation of Concerns**: Transport layer is separate from protocol layer
2. **Extensibility**: Protocol interface allows easy extension
3. **Runtime Flexibility**: Can switch between TCP and UDP at runtime
4. **Resource Management**: Uses smart pointers for automatic cleanup

## License

This library is provided as-is for educational and development purposes.

