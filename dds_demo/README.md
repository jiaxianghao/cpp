# FastDDS Wrapper - Easy DDS Communication Framework

A user-friendly C++ wrapper for FastDDS (eProsima Fast DDS) that simplifies DDS communication for developers who are not familiar with DDS concepts.

## Features

- **Simple API** - Publish and subscribe with just 3 lines of code
- **Zero-Intrusion Extension** - Add new message types without modifying wrapper code
- **Template-Based Design** - Type-safe compile-time checking
- **Auto Resource Management** - RAII-based resource handling
- **Configuration Management** - JSON and XML configuration support
- **Connection Monitoring** - Auto-reconnect and health monitoring
- **Comprehensive Logging** - Multi-level logging system
- **Rich Examples** - Learn by example with detailed tutorials

## Quick Start

### Prerequisites

- CMake 3.16+
- C++17 compatible compiler
- FastDDS 2.10+ installed
- FastDDS-Gen tool

### Build

```bash
mkdir build && cd build
cmake ..
make
```

### Basic Usage

```cpp
#include <dds_wrapper/DDSManager.h>
#include "CommonMessage.h"

int main()
{
    // Initialize DDS Manager
    DDSManager manager;
    manager.initialize("config.json");

    // Create Publisher
    auto pub = manager.createPublisher<CommonMessage>("TopicA");
    
    // Publish message
    CommonMessage msg;
    msg.content("Hello from FastDDS!");
    pub->publish(msg);

    // Create Subscriber with callback
    auto sub = manager.createSubscriber<CommonMessage>("TopicA",
        [](const CommonMessage& msg)
        {
            std::cout << "Received: " << msg.content() << std::endl;
        });

    return 0;
}
```

## Adding New Message Types

### Step 1: Define IDL (1 minute)

Create `idl/MyMessage.idl`:

```idl
struct MyMessage
{
    string id;
    long value;
    double timestamp;
};
```

### Step 2: Build (automatic)

```bash
cd build
cmake ..
make
```

### Step 3: Use in Code

```cpp
auto pub = manager.createPublisher<MyMessage>("MyTopic");
MyMessage msg;
msg.id("MSG_001");
msg.value(42);
pub->publish(msg);
```

That's it! No wrapper modification needed.

## Documentation

- [Quick Start Guide](docs/QUICK_START.md)
- [API Reference](docs/API_REFERENCE.md)
- [Configuration Guide](docs/CONFIGURATION.md)
- [Adding New Types](docs/ADD_NEW_TYPE.md)
- [IDL Guide](docs/IDL_GUIDE.md)
- [Troubleshooting](docs/TROUBLESHOOTING.md)

## Project Structure

```
dds_demo/
├── include/dds_wrapper/   # Header files
├── src/                   # Implementation files
├── idl/                   # IDL message definitions
├── config/                # Configuration templates
├── examples/              # Usage examples
├── tools/                 # Helper tools
├── docs/                  # Documentation
└── CMakeLists.txt         # Build configuration
```

## Examples

Run the examples to learn how to use the framework:

```bash
# Basic publish-subscribe
./build/bin/examples/basic_pubsub

# Multiple topics
./build/bin/examples/multiple_topics

# Connection monitoring
./build/bin/examples/monitoring

# See all available message types
./build/bin/tools/list_types
```

## License

MIT License - See LICENSE file for details

## Contributing

Contributions are welcome! Please read CONTRIBUTING.md for details.

## Support

For issues and questions, please open an issue on GitHub.
