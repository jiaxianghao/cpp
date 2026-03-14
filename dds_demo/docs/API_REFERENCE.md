# API Reference

Complete API documentation for FastDDS Wrapper.

## Core Classes

### DDSManager

Main entry point for DDS communication.

#### Constructor

```cpp
DDSManager();
```

Creates a new DDSManager instance (not initialized).

#### Methods

##### initialize

```cpp
bool initialize(const std::string& config_file = "");
bool initialize(const DDSConfig& config);
```

Initializes the DDS domain participant and communication infrastructure.

**Parameters:**
- `config_file`: Path to JSON configuration file (optional)
- `config`: DDSConfig object with settings

**Returns:** `true` on success, `false` on failure

**Example:**
```cpp
DDSManager manager;
manager.initialize("config.json");
// or
DDSConfig config;
config.domain_id = 5;
manager.initialize(config);
```

##### shutdown

```cpp
void shutdown();
```

Cleans up all DDS resources. Called automatically by destructor.

**Example:**
```cpp
manager.shutdown();
```

##### createPublisher

```cpp
template<typename T>
std::shared_ptr<Publisher<T>> createPublisher(const std::string& topic_name);
```

Creates a publisher for the specified message type and topic.

**Template Parameters:**
- `T`: Message type (must be generated from IDL)

**Parameters:**
- `topic_name`: Name of the topic to publish on

**Returns:** Shared pointer to Publisher instance

**Throws:** `InitializationException`, `PublisherException`

**Example:**
```cpp
auto pub = manager.createPublisher<CommonMessage>("MyTopic");
```

##### createSubscriber

```cpp
template<typename T>
std::shared_ptr<Subscriber<T>> createSubscriber(
    const std::string& topic_name,
    MessageCallback<T> callback
);
```

Creates a subscriber for the specified message type and topic.

**Template Parameters:**
- `T`: Message type (must be generated from IDL)

**Parameters:**
- `topic_name`: Name of the topic to subscribe to
- `callback`: Function called when messages arrive

**Returns:** Shared pointer to Subscriber instance

**Throws:** `InitializationException`, `SubscriberException`

**Example:**
```cpp
auto sub = manager.createSubscriber<CommonMessage>("MyTopic",
    [](const CommonMessage& msg)
    {
        std::cout << "Received: " << msg.content() << std::endl;
    });
```

##### getStatus

```cpp
ConnectionStatus getStatus() const;
```

Returns the current connection status.

**Returns:** ConnectionStatus enum value

**Example:**
```cpp
if (manager.getStatus() == ConnectionStatus::CONNECTED)
{
    // Ready to communicate
}
```

##### isInitialized

```cpp
bool isInitialized() const;
```

Checks if the manager is initialized.

**Returns:** `true` if initialized

##### getConfigManager

```cpp
ConfigManager& getConfigManager();
```

Returns reference to configuration manager.

**Returns:** Reference to ConfigManager

**Example:**
```cpp
auto& config_mgr = manager.getConfigManager();
config_mgr.setDomainId(5);
```

---

### Publisher<T>

Template class for publishing messages.

#### Methods

##### publish

```cpp
bool publish(const T& data);
```

Publishes a message to the topic.

**Parameters:**
- `data`: Message to publish

**Returns:** `true` on success, `false` on failure

**Thread Safety:** Thread-safe

**Example:**
```cpp
CommonMessage msg;
msg.content("Hello");
if (pub->publish(msg))
{
    std::cout << "Published successfully" << std::endl;
}
```

##### isConnected

```cpp
bool isConnected() const;
```

Checks if publisher has matched subscribers.

**Returns:** `true` if connected to at least one subscriber

##### getTopicName

```cpp
std::string getTopicName() const;
```

Returns the topic name.

**Returns:** Topic name string

---

### Subscriber<T>

Template class for receiving messages.

#### Methods

##### start

```cpp
void start();
```

Starts receiving messages. Called automatically on creation.

##### stop

```cpp
void stop();
```

Stops receiving messages.

**Example:**
```cpp
sub->stop();  // Temporarily pause
// ... do something ...
sub->start();  // Resume
```

##### isRunning

```cpp
bool isRunning() const;
```

Checks if subscriber is actively receiving.

**Returns:** `true` if running

##### setCallback

```cpp
void setCallback(MessageCallback<T> callback);
```

Changes the message callback.

**Parameters:**
- `callback`: New callback function

**Thread Safety:** Thread-safe

**Example:**
```cpp
sub->setCallback([](const CommonMessage& msg)
{
    // New handler
});
```

##### getTopicName

```cpp
std::string getTopicName() const;
```

Returns the topic name.

**Returns:** Topic name string

---

### ConfigManager

Manages DDS configuration.

#### Methods

##### loadFromJSON

```cpp
bool loadFromJSON(const std::string& json_file);
```

Loads configuration from JSON file.

**Parameters:**
- `json_file`: Path to JSON configuration

**Returns:** `true` on success

##### saveToJSON

```cpp
bool saveToJSON(const std::string& json_file) const;
```

Saves current configuration to JSON file.

**Parameters:**
- `json_file`: Path to save configuration

**Returns:** `true` on success

##### getConfig

```cpp
const DDSConfig& getConfig() const;
```

Returns current configuration.

**Returns:** Reference to DDSConfig

##### setConfig

```cpp
void setConfig(const DDSConfig& config);
```

Sets new configuration.

**Parameters:**
- `config`: New configuration

##### setDomainId

```cpp
void setDomainId(int domain_id);
```

Sets the DDS domain ID.

**Parameters:**
- `domain_id`: Domain ID (0-232)

##### setReliability

```cpp
void setReliability(ReliabilityKind reliability);
```

Sets QoS reliability.

**Parameters:**
- `reliability`: ReliabilityKind::RELIABLE or BEST_EFFORT

##### setDurability

```cpp
void setDurability(DurabilityKind durability);
```

Sets QoS durability.

**Parameters:**
- `durability`: DurabilityKind enum value

##### setHistoryDepth

```cpp
void setHistoryDepth(int depth);
```

Sets history buffer depth.

**Parameters:**
- `depth`: Number of messages to buffer

---

### Logger

Singleton logging system.

#### Methods

##### getInstance

```cpp
static Logger& getInstance();
```

Returns logger singleton instance.

**Returns:** Reference to Logger

##### initialize

```cpp
void initialize(LogLevel level, bool console_output, bool file_output, const std::string& log_dir);
```

Initializes the logging system.

**Parameters:**
- `level`: Minimum log level
- `console_output`: Enable console output
- `file_output`: Enable file output
- `log_dir`: Directory for log files

##### setLogLevel

```cpp
void setLogLevel(LogLevel level);
```

Changes the log level.

**Parameters:**
- `level`: New log level

##### debug, info, warn, error

```cpp
void debug(const std::string& message);
void info(const std::string& message);
void warn(const std::string& message);
void error(const std::string& message);
```

Log messages at specific levels.

**Parameters:**
- `message`: Log message

**Example:**
```cpp
Logger::getInstance().info("Application started");
Logger::getInstance().error("Connection failed");
```

---

### ConnectionMonitor

Monitors DDS connection health.

#### Methods

##### start

```cpp
void start(int heartbeat_interval_ms);
```

Starts monitoring.

**Parameters:**
- `heartbeat_interval_ms`: Heartbeat check interval

##### stop

```cpp
void stop();
```

Stops monitoring.

##### setStatusCallback

```cpp
void setStatusCallback(ConnectionStatusCallback callback);
```

Sets callback for status changes.

**Parameters:**
- `callback`: Function called on status change

**Example:**
```cpp
monitor.setStatusCallback([](ConnectionStatus status)
{
    std::cout << "Status changed" << std::endl;
});
```

##### getStatus

```cpp
ConnectionStatus getStatus() const;
```

Returns current connection status.

**Returns:** ConnectionStatus enum

---

## Data Types

### DDSConfig

Configuration structure.

```cpp
struct DDSConfig
{
    int domain_id = 0;
    std::string participant_name = "DDSWrapper_Participant";
    
    ReliabilityKind reliability = ReliabilityKind::RELIABLE;
    DurabilityKind durability = DurabilityKind::TRANSIENT_LOCAL;
    int history_depth = 10;
    
    bool monitoring_enabled = true;
    int heartbeat_interval_ms = 1000;
    bool auto_reconnect = true;
    int max_reconnect_attempts = 5;
    
    LogLevel log_level = LogLevel::INFO;
    bool log_file_output = true;
    std::string log_dir = "logs";
    bool log_console_output = true;
    
    std::map<std::string, std::string> custom_properties;
};
```

### Enumerations

#### ConnectionStatus

```cpp
enum class ConnectionStatus
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR
};
```

#### ReliabilityKind

```cpp
enum class ReliabilityKind
{
    BEST_EFFORT,  // Fire-and-forget
    RELIABLE      // Guaranteed delivery
};
```

#### DurabilityKind

```cpp
enum class DurabilityKind
{
    VOLATILE,         // No history
    TRANSIENT_LOCAL,  // Local history
    TRANSIENT,        // Persistent history
    PERSISTENT        // Database-backed
};
```

#### LogLevel

```cpp
enum class LogLevel
{
    DEBUG,
    INFO,
    WARN,
    ERROR
};
```

#### ErrorCode

```cpp
enum class ErrorCode
{
    SUCCESS,
    INIT_FAILED,
    ALREADY_INITIALIZED,
    NOT_INITIALIZED,
    INVALID_CONFIG,
    TOPIC_CREATE_FAILED,
    PUBLISHER_CREATE_FAILED,
    SUBSCRIBER_CREATE_FAILED,
    PARTICIPANT_CREATE_FAILED,
    PUBLISH_FAILED,
    CONNECTION_LOST,
    TIMEOUT,
    UNKNOWN_ERROR
};
```

---

## Exceptions

### DDSException

Base exception class.

```cpp
class DDSException : public std::exception
{
public:
    ErrorCode getErrorCode() const;
    std::string getMessage() const;
};
```

### InitializationException

```cpp
class InitializationException : public DDSException { };
```

Thrown when initialization fails.

### ConfigurationException

```cpp
class ConfigurationException : public DDSException { };
```

Thrown for configuration errors.

### TopicException

```cpp
class TopicException : public DDSException { };
```

Thrown when topic creation fails.

### PublisherException

```cpp
class PublisherException : public DDSException { };
```

Thrown for publisher errors.

### SubscriberException

```cpp
class SubscriberException : public DDSException { };
```

Thrown for subscriber errors.

### CommunicationException

```cpp
class CommunicationException : public DDSException { };
```

Thrown for communication failures.

---

## Type Aliases

```cpp
template<typename T>
using MessageCallback = std::function<void(const T&)>;

using ConnectionStatusCallback = std::function<void(ConnectionStatus)>;
```

---

## Complete Usage Example

```cpp
#include "dds_wrapper/DDSManager.h"
#include "CommonMessage.h"
#include <iostream>

using namespace dds_wrapper;

int main()
{
    try
    {
        // Initialize with custom config
        DDSConfig config;
        config.domain_id = 0;
        config.reliability = ReliabilityKind::RELIABLE;
        config.log_level = LogLevel::DEBUG;
        
        DDSManager manager;
        if (!manager.initialize(config))
        {
            std::cerr << "Initialization failed" << std::endl;
            return 1;
        }
        
        // Create subscriber
        auto sub = manager.createSubscriber<CommonMessage>("MyTopic",
            [](const CommonMessage& msg)
            {
                std::cout << "Received: " << msg.content() << std::endl;
            });
        
        // Create publisher
        auto pub = manager.createPublisher<CommonMessage>("MyTopic");
        
        // Wait for discovery
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Publish message
        CommonMessage msg;
        msg.message_id("MSG_001");
        msg.content("Hello FastDDS!");
        msg.timestamp(std::chrono::system_clock::now().time_since_epoch().count());
        msg.sequence_number(1);
        
        if (pub->publish(msg))
        {
            std::cout << "Published successfully" << std::endl;
        }
        
        // Check status
        if (manager.getStatus() == ConnectionStatus::CONNECTED)
        {
            std::cout << "Connected and operational" << std::endl;
        }
        
        // Wait for message processing
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Cleanup (automatic on destruction)
        manager.shutdown();
    }
    catch (const DDSException& e)
    {
        std::cerr << "DDS Error: " << e.getMessage() << std::endl;
        std::cerr << "Error code: " << static_cast<int>(e.getErrorCode()) << std::endl;
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

---

## Thread Safety

- `DDSManager`: Thread-safe
- `Publisher::publish()`: Thread-safe
- `Subscriber` callbacks: Run in DDS thread, keep processing minimal
- `ConfigManager`: Not thread-safe, configure before creating publishers/subscribers
- `Logger`: Thread-safe

---

## See Also

- [Quick Start Guide](QUICK_START.md)
- [Configuration Guide](CONFIGURATION.md)
- [Adding New Types](ADD_NEW_TYPE.md)
- [Troubleshooting](TROUBLESHOOTING.md)
