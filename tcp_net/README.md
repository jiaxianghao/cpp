# TCP Network Library

Enhanced TCP communication library for network applications.

## Features

- Enhanced TCP socket communication with connection management
- Automatic reconnection and error handling
- Heartbeat monitoring for connection health
- Thread-safe command queuing
- Configurable connection parameters
- Cross-platform support (Linux, macOS, Windows)

## Building with CMake

### Prerequisites

- CMake 3.16 or higher
- C++17 compatible compiler
- Threads library (usually included with the compiler)

### Build Instructions

#### Option 1: Using the build script (Recommended)
```bash
# Build the project
./build.sh

# Build and run the example
./build.sh --run-example

# Clean build files
./clean.sh
```

#### Option 2: Manual build
1. **Clone and navigate to the project directory:**
   ```bash
   cd net
   ```

2. **Create build directory:**
   ```bash
   mkdir build
   cd build
   ```

3. **Configure the project:**
   ```bash
   cmake ..
   ```

4. **Build the project:**
   ```bash
   make
   ```

5. **Run the example:**
   ```bash
   ./bin/enhanced_tcp_example
   ```

### Build Options

- **CMAKE_BUILD_TYPE**: Set to `Debug` or `Release` (default: `Release`)
- **CMAKE_INSTALL_PREFIX**: Installation directory (default: `/usr/local`)

Example:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/opt/tcp_network_lib ..
```



### Installation

To install the library system-wide:
```bash
make install
```

This will install:
- Headers to `${CMAKE_INSTALL_PREFIX}/include/tcp_network_lib/`
- Library to `${CMAKE_INSTALL_PREFIX}/lib/`
- Example binary to `${CMAKE_INSTALL_PREFIX}/bin/`

## Usage

### Basic Usage

```cpp
#include "tcp_communicator.h"

// Create communicator
auto communicator = std::make_unique<TcpCommunicatorImpl>();

// Configure connection
ConnectionConfig config;
config.connect_timeout_ms = 5000;
config.retry_count = 3;
communicator->set_connection_config(config);

// Connect to server
if (communicator->connect_to_server("192.168.1.100", 8080)) {
    // Send command
    communicator->send_command("#[GET]\n");
    
    // Register callback for responses
    communicator->register_global_callback([](const std::string& response) {
        std::cout << "Received: " << response << std::endl;
    });
    
    // Start heartbeat monitoring
    communicator->start_heartbeat(30000);
    
    // Main loop
    while (communicator->is_connected()) {
        communicator->update();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
```

### Connection Configuration

```cpp
ConnectionConfig config;
config.connect_timeout_ms = 5000;    // Connection timeout
config.read_timeout_ms = 3000;       // Read/write timeout
config.retry_count = 3;              // Retry attempts
config.retry_interval_ms = 1000;     // Retry interval
config.enable_keepalive = true;      // Enable TCP keepalive
config.keepalive_idle = 60;          // Keepalive idle time
config.keepalive_interval = 10;      // Keepalive probe interval
config.keepalive_count = 3;          // Max keepalive probes
```

## Project Structure

```
net/
├── CMakeLists.txt              # Main CMake configuration
├── src/
│   ├── CMakeLists.txt          # Library build configuration
│   ├── tcp_communicator.h      # Main header file
│   ├── tcp_communicator.cc     # Implementation
│   └── compatibility.h         # Compatibility layer
├── examples/
│   ├── CMakeLists.txt          # Examples build configuration
│   └── enhanced_tcp_example.cc # Usage example
├── cmake/
│   └── tcp_network_libConfig.cmake.in  # Package config template
└── README.md                   # This file
```

## API Reference

### Main Classes

- **TcpCommunicatorImpl**: Main TCP communication class
- **ITcpCommunicator**: Base interface for TCP communication
- **ConnectionConfig**: Configuration structure for connection parameters
- **ConnectionResult**: Enumeration for connection result codes

### Key Methods

- `connect_to_server(ip, port)`: Connect to a server
- `disconnect_from_server()`: Disconnect from server
- `send_command(command)`: Send a command asynchronously
- `register_global_callback(callback)`: Register response callback
- `is_connected()`: Check connection status
- `is_connection_healthy()`: Check connection health
- `start_heartbeat(interval_ms)`: Start heartbeat monitoring
- `update()`: Process pending operations

## License

Copyright 2016-2024. UISEE TECHNOLOGIES (BEIJING) LTD. All rights reserved.
