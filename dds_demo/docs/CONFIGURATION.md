# Configuration Guide

Complete guide to configuring the FastDDS Wrapper.

## Configuration Methods

### 1. Default Configuration

Use default settings:

```cpp
DDSManager manager;
manager.initialize();  // Uses built-in defaults
```

### 2. JSON Configuration File

Create `config.json`:

```json
{
    "domain_id": 0,
    "participant_name": "MyApplication",
    "qos": {
        "reliability": "reliable",
        "durability": "transient_local",
        "history_depth": 10
    },
    "monitoring": {
        "enabled": true,
        "heartbeat_interval_ms": 1000,
        "auto_reconnect": true,
        "max_reconnect_attempts": 5
    },
    "logging": {
        "level": "INFO",
        "file_output": true,
        "log_dir": "logs",
        "console_output": true
    }
}
```

Load it:

```cpp
manager.initialize("config.json");
```

### 3. Programmatic Configuration

```cpp
DDSConfig config;
config.domain_id = 0;
config.participant_name = "MyApp";
config.reliability = ReliabilityKind::RELIABLE;
config.durability = DurabilityKind::TRANSIENT_LOCAL;
config.history_depth = 20;

manager.initialize(config);
```

## Configuration Options

### Domain Settings

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `domain_id` | int | 0 | DDS domain ID (0-232) |
| `participant_name` | string | "DDSWrapper_Participant" | Unique participant name |

```cpp
config.domain_id = 5;
config.participant_name = "Robot_Controller";
```

### QoS Settings

#### Reliability

Controls message delivery guarantees:

| Value | Description | Use Case |
|-------|-------------|----------|
| `BEST_EFFORT` | Fire-and-forget | High-frequency data (sensors) |
| `RELIABLE` | Guaranteed delivery | Commands, critical data |

```cpp
config.reliability = ReliabilityKind::RELIABLE;
```

JSON:
```json
"reliability": "reliable"  // or "best_effort"
```

#### Durability

Controls historical data availability:

| Value | Description | Use Case |
|-------|-------------|----------|
| `VOLATILE` | No history | Real-time only |
| `TRANSIENT_LOCAL` | Local history | Late joiners get history |
| `TRANSIENT` | Persistent history | Survives restarts |
| `PERSISTENT` | Database-backed | Long-term storage |

```cpp
config.durability = DurabilityKind::TRANSIENT_LOCAL;
```

#### History Depth

Number of messages to keep in history:

```cpp
config.history_depth = 100;  // Keep last 100 messages
```

### Monitoring Settings

#### Heartbeat Monitoring

```cpp
config.monitoring_enabled = true;
config.heartbeat_interval_ms = 1000;  // Check every second
```

#### Auto-Reconnect

```cpp
config.auto_reconnect = true;
config.max_reconnect_attempts = 5;
```

JSON:
```json
"monitoring": {
    "enabled": true,
    "heartbeat_interval_ms": 1000,
    "auto_reconnect": true,
    "max_reconnect_attempts": 5
}
```

### Logging Settings

#### Log Levels

| Level | Description |
|-------|-------------|
| `DEBUG` | Detailed debugging info |
| `INFO` | General information |
| `WARN` | Warnings |
| `ERROR` | Errors only |

```cpp
config.log_level = LogLevel::DEBUG;
config.log_console_output = true;
config.log_file_output = true;
config.log_dir = "logs";
```

JSON:
```json
"logging": {
    "level": "DEBUG",
    "file_output": true,
    "log_dir": "logs",
    "console_output": true
}
```

## Configuration Examples

### High-Performance Sensor Data

```cpp
DDSConfig config;
config.reliability = ReliabilityKind::BEST_EFFORT;
config.durability = DurabilityKind::VOLATILE;
config.history_depth = 1;  // Only latest value
```

### Reliable Command Channel

```cpp
DDSConfig config;
config.reliability = ReliabilityKind::RELIABLE;
config.durability = DurabilityKind::TRANSIENT_LOCAL;
config.history_depth = 100;
config.auto_reconnect = true;
```

### Debug Configuration

```cpp
DDSConfig config;
config.log_level = LogLevel::DEBUG;
config.log_console_output = true;
config.log_file_output = true;
config.monitoring_enabled = true;
```

## Per-Topic Configuration

For different QoS per topic, use ConfigManager:

```cpp
DDSManager manager;
manager.initialize();

// Get config manager
auto& config_mgr = manager.getConfigManager();

// Change settings for next topic
config_mgr.setReliability(ReliabilityKind::BEST_EFFORT);
config_mgr.setHistoryDepth(1);

// Create publisher with these settings
auto pub = manager.createPublisher<SensorData>("FastSensorTopic");

// Change settings again
config_mgr.setReliability(ReliabilityKind::RELIABLE);
auto cmd_pub = manager.createPublisher<CommandMessage>("CommandTopic");
```

## Environment Variables

Override configuration via environment:

```bash
export DDS_DOMAIN_ID=5
export DDS_LOG_LEVEL=DEBUG
```

## XML Configuration (Advanced)

For advanced FastDDS features, use XML profiles:

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<profiles xmlns="http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles">
    <participant profile_name="custom_participant">
        <rtps>
            <builtin>
                <discovery_config>
                    <leaseDuration>
                        <sec>20</sec>
                    </leaseDuration>
                </discovery_config>
            </builtin>
        </rtps>
    </participant>
</profiles>
```

Load via FastDDS environment:
```bash
export FASTRTPS_DEFAULT_PROFILES_FILE=config/profiles.xml
```

## Configuration File Locations

Default search path:

1. Path specified in `initialize(path)`
2. `./config.json`
3. `../config/default_config.json`
4. Built-in defaults

## Saving Configuration

Save current config to file:

```cpp
manager.getConfigManager().saveToJSON("my_config.json");
```

## Best Practices

### 1. Start with Defaults

```cpp
// Test with defaults first
manager.initialize();
```

### 2. Use JSON for Production

```cpp
// Production: external config
manager.initialize("config.json");
```

### 3. Programmatic for Tests

```cpp
// Unit tests: explicit config
DDSConfig config;
config.domain_id = 100;  // Test domain
manager.initialize(config);
```

### 4. Log Configuration

```cpp
config.log_level = LogLevel::DEBUG;  // Development
config.log_level = LogLevel::WARN;   // Production
```

## Troubleshooting

### Messages Not Delivered

Try:
```cpp
config.reliability = ReliabilityKind::RELIABLE;
config.history_depth = 100;
```

### Late Joiners Missing Data

Use:
```cpp
config.durability = DurabilityKind::TRANSIENT_LOCAL;
```

### High CPU Usage

Reduce:
```cpp
config.heartbeat_interval_ms = 5000;  // Less frequent
```

### Large Message Buffers

Increase:
```cpp
config.history_depth = 1000;
```

## Summary

```cpp
// Quick setup
manager.initialize();  // Defaults

// Production
manager.initialize("config.json");  // File-based

// Custom
DDSConfig config;
// ... configure ...
manager.initialize(config);  // Programmatic
```
