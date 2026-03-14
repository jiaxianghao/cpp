# Quick Start Guide

Get started with FastDDS Wrapper in 5 minutes!

## Prerequisites

- CMake 3.16 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- FastDDS 2.10+ installed
- FastDDS-Gen tool

## Installation

### 1. Clone or download the project

```bash
cd dds_demo
```

### 2. Build the project

```bash
mkdir build && cd build
cmake ..
make
```

## Your First DDS Application

### Step 1: Initialize DDSManager

```cpp
#include "dds_wrapper/DDSManager.h"
#include "CommonMessage.h"

using namespace dds_wrapper;

int main()
{
    DDSManager manager;
    manager.initialize();  // Uses default config
    
    return 0;
}
```

### Step 2: Create a Publisher

```cpp
// Create publisher for CommonMessage type
auto publisher = manager.createPublisher<CommonMessage>("MyTopic");

// Create and publish a message
CommonMessage msg;
msg.message_id("MSG_001");
msg.content("Hello FastDDS!");
msg.timestamp(std::chrono::system_clock::now().time_since_epoch().count());
msg.sequence_number(1);

publisher->publish(msg);
```

### Step 3: Create a Subscriber

```cpp
// Create subscriber with callback
auto subscriber = manager.createSubscriber<CommonMessage>("MyTopic",
    [](const CommonMessage& msg)
    {
        std::cout << "Received: " << msg.content() << std::endl;
    });
```

## Complete Example

```cpp
#include "dds_wrapper/DDSManager.h"
#include "CommonMessage.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace dds_wrapper;

int main()
{
    try
    {
        // Initialize
        DDSManager manager;
        if (!manager.initialize())
        {
            std::cerr << "Failed to initialize" << std::endl;
            return 1;
        }

        // Create subscriber
        auto sub = manager.createSubscriber<CommonMessage>("HelloTopic",
            [](const CommonMessage& msg)
            {
                std::cout << "Got: " << msg.content() << std::endl;
            });

        // Create publisher
        auto pub = manager.createPublisher<CommonMessage>("HelloTopic");

        // Wait for discovery
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Send message
        CommonMessage msg;
        msg.message_id("HELLO");
        msg.content("Hello World!");
        msg.timestamp(0);
        msg.sequence_number(1);
        
        pub->publish(msg);

        // Wait for message to be received
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    catch (const DDSException& e)
    {
        std::cerr << "Error: " << e.getMessage() << std::endl;
        return 1;
    }

    return 0;
}
```

## Run Examples

The project includes several examples:

```bash
# Basic publish-subscribe
./build/bin/examples/basic_pubsub

# Multiple topics
./build/bin/examples/multiple_topics

# Reliable communication
./build/bin/examples/reliable_communication

# Connection monitoring
./build/bin/examples/monitoring

# Custom QoS
./build/bin/examples/custom_qos
```

## Configuration

### Using JSON Configuration

Create `config.json`:

```json
{
    "domain_id": 0,
    "participant_name": "MyApp",
    "qos": {
        "reliability": "reliable",
        "durability": "transient_local",
        "history_depth": 10
    }
}
```

Load it:

```cpp
manager.initialize("config.json");
```

## Next Steps

- [Adding New Message Types](ADD_NEW_TYPE.md)
- [API Reference](API_REFERENCE.md)
- [Configuration Guide](CONFIGURATION.md)
- [Troubleshooting](TROUBLESHOOTING.md)

## Common Issues

### FastDDS not found

```bash
# On Ubuntu/Debian
sudo apt install libfastrtps-dev libfastcdr-dev

# Or build from source
# See: https://fast-dds.docs.eprosima.com/
```

### fastddsgen not found

```bash
# Download and install FastDDS-Gen
# See: https://github.com/eProsima/Fast-DDS-Gen
```

## Getting Help

- Check the [Troubleshooting Guide](TROUBLESHOOTING.md)
- Review the examples in `examples/`
- Read the [API Reference](API_REFERENCE.md)
