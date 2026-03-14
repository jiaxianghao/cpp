# IDL Syntax Guide

Complete reference for DDS IDL (Interface Definition Language).

## Overview

IDL defines the structure of messages exchanged via DDS. The wrapper automatically generates C++ code from IDL files.

## Basic Syntax

### Struct Definition

```idl
struct MessageName
{
    type_name field_name;
    // ... more fields ...
};
```

### Example

```idl
struct SensorReading
{
    string sensor_id;
    double value;
    long timestamp;
};
```

## Primitive Types

| IDL Type | C++ Type | Size | Range |
|----------|----------|------|-------|
| `boolean` | `bool` | 1 byte | true/false |
| `char` | `char` | 1 byte | -128 to 127 |
| `octet` | `uint8_t` | 1 byte | 0 to 255 |
| `short` | `int16_t` | 2 bytes | -32,768 to 32,767 |
| `unsigned short` | `uint16_t` | 2 bytes | 0 to 65,535 |
| `long` | `int32_t` | 4 bytes | -2³¹ to 2³¹-1 |
| `unsigned long` | `uint32_t` | 4 bytes | 0 to 2³²-1 |
| `long long` | `int64_t` | 8 bytes | -2⁶³ to 2⁶³-1 |
| `unsigned long long` | `uint64_t` | 8 bytes | 0 to 2⁶⁴-1 |
| `float` | `float` | 4 bytes | IEEE 754 single |
| `double` | `double` | 8 bytes | IEEE 754 double |
| `string` | `std::string` | Variable | Text data |

### Example

```idl
struct AllTypes
{
    boolean active;
    char initial;
    octet byte_data;
    short temperature;
    unsigned short port;
    long count;
    unsigned long id;
    long long big_number;
    unsigned long long timestamp;
    float ratio;
    double precise_value;
    string name;
};
```

## Strings

### Unbounded String

```idl
struct Message
{
    string text;  // Any length
};
```

### Bounded String

```idl
struct LimitedMessage
{
    string<128> text;  // Max 128 characters
};
```

## Arrays

### Fixed-Size Array

```idl
struct VectorData
{
    double coordinates[3];  // Exactly 3 elements
    long matrix[3][3];      // 3x3 matrix
};
```

Usage in C++:
```cpp
VectorData data;
data.coordinates(0, 1.0);
data.coordinates(1, 2.0);
data.coordinates(2, 3.0);
```

## Sequences

### Unbounded Sequence

```idl
struct DataSet
{
    sequence<double> values;  // Any number of elements
};
```

### Bounded Sequence

```idl
struct LimitedDataSet
{
    sequence<double, 100> values;  // Max 100 elements
};
```

Usage in C++:
```cpp
DataSet data;
std::vector<double> vals = {1.0, 2.0, 3.0};
data.values(vals);
```

## Enumerations

### Basic Enum

```idl
enum Status
{
    IDLE,
    RUNNING,
    PAUSED,
    STOPPED
};

struct DeviceState
{
    string device_id;
    Status current_status;
};
```

Usage in C++:
```cpp
DeviceState state;
state.device_id("DEV_001");
state.current_status(Status::RUNNING);
```

### Enum with Explicit Values

```idl
enum ErrorCode
{
    SUCCESS = 0,
    WARNING = 1,
    ERROR = 2,
    CRITICAL = 3
};
```

## Nested Structures

```idl
struct Position
{
    double x;
    double y;
    double z;
};

struct Velocity
{
    double vx;
    double vy;
    double vz;
};

struct RobotState
{
    string robot_id;
    Position position;
    Velocity velocity;
    long timestamp;
};
```

Usage in C++:
```cpp
RobotState state;
state.robot_id("ROBOT_001");
state.position().x(10.0);
state.position().y(20.0);
state.position().z(0.0);
state.velocity().vx(1.5);
```

## Unions

```idl
union SensorValue switch(long)
{
    case 0:
        double temperature;
    case 1:
        long pressure;
    case 2:
        boolean status;
    default:
        string raw_value;
};

struct GenericSensor
{
    string sensor_id;
    SensorValue value;
};
```

Usage in C++:
```cpp
GenericSensor sensor;
sensor.sensor_id("TEMP_001");
sensor.value()._d(0);  // Select temperature
sensor.value().temperature(25.5);
```

## Optional Fields

```idl
struct ExtendedMessage
{
    string id;
    @optional string description;
    @optional double optional_value;
};
```

## Maps

```idl
struct Configuration
{
    string config_id;
    map<string, string> parameters;
    map<string, long> limits;
};
```

Usage in C++:
```cpp
Configuration config;
config.config_id("CFG_001");
std::map<std::string, std::string> params;
params["timeout"] = "5000";
params["retries"] = "3";
config.parameters(params);
```

## Constants

```idl
const long MAX_BUFFER_SIZE = 1024;
const string DEFAULT_NAME = "Unnamed";
const double PI = 3.14159265359;

struct BufferConfig
{
    long buffer_size;  // Can use MAX_BUFFER_SIZE in code
    string name;
};
```

## Modules (Namespaces)

```idl
module sensors
{
    struct TemperatureData
    {
        double celsius;
        long timestamp;
    };
    
    struct PressureData
    {
        double pascals;
        long timestamp;
    };
};

module actuators
{
    struct MotorCommand
    {
        long motor_id;
        double speed;
    };
};
```

Usage in C++:
```cpp
sensors::TemperatureData temp;
temp.celsius(25.0);

actuators::MotorCommand cmd;
cmd.motor_id(1);
```

## Annotations

### Key Fields (for filtering)

```idl
struct KeyedMessage
{
    @key string device_id;  // Key field
    double value;
    long timestamp;
};
```

### Default Values

```idl
struct ConfigWithDefaults
{
    @default(10) long timeout;
    @default("default") string name;
};
```

## Complex Examples

### Vehicle Tracking System

```idl
enum VehicleType
{
    CAR,
    TRUCK,
    BUS,
    MOTORCYCLE
};

struct GPSCoordinate
{
    double latitude;
    double longitude;
    double altitude;
};

struct VehicleState
{
    @key string vehicle_id;
    VehicleType type;
    GPSCoordinate location;
    double speed;
    double heading;
    long timestamp;
};

struct Route
{
    string route_id;
    sequence<GPSCoordinate> waypoints;
    double total_distance;
};
```

### Industrial Sensor Network

```idl
enum SensorType
{
    TEMPERATURE,
    PRESSURE,
    HUMIDITY,
    VIBRATION,
    CURRENT
};

struct SensorReading
{
    @key string sensor_id;
    SensorType type;
    double value;
    string unit;
    boolean is_valid;
    long timestamp;
    @optional string error_message;
};

struct SensorConfig
{
    @key string sensor_id;
    double sampling_rate;
    double threshold_min;
    double threshold_max;
    map<string, string> calibration_data;
};
```

### Smart Home System

```idl
enum DeviceCategory
{
    LIGHTING,
    HVAC,
    SECURITY,
    APPLIANCE
};

enum CommandType
{
    TURN_ON,
    TURN_OFF,
    SET_VALUE,
    GET_STATUS
};

struct DeviceCommand
{
    string command_id;
    string device_id;
    CommandType type;
    @optional double value;
    @optional string parameter;
    long timestamp;
};

struct DeviceStatus
{
    @key string device_id;
    DeviceCategory category;
    boolean is_online;
    boolean is_active;
    double current_value;
    map<string, string> properties;
    long last_update;
};
```

## Best Practices

### 1. Always Include ID and Timestamp

```idl
struct BestPracticeMessage
{
    string message_id;     // Unique identifier
    // ... your fields ...
    long timestamp;        // When created
};
```

### 2. Use Meaningful Names

```idl
// Good
struct TemperatureSensorReading { ... }

// Avoid
struct Data1 { ... }
struct Msg { ... }
```

### 3. Add Comments

```idl
// Temperature reading from DHT22 sensor
// Units: Celsius
// Range: -40 to 80
struct TemperatureReading
{
    string sensor_id;      // Unique sensor identifier
    double temperature;    // Temperature in Celsius
    double humidity;       // Relative humidity (0-100%)
    long timestamp;        // Unix timestamp in milliseconds
};
```

### 4. Group Related Fields

```idl
struct DeviceInfo
{
    // Identification
    string device_id;
    string device_name;
    string manufacturer;
    
    // Status
    boolean is_online;
    boolean is_active;
    
    // Metrics
    double temperature;
    double voltage;
    
    // Metadata
    long last_update;
    long uptime;
};
```

### 5. Use Enums for Fixed Sets

```idl
// Good
enum ConnectionState { DISCONNECTED, CONNECTING, CONNECTED, ERROR };

// Avoid using strings for fixed sets
// string state;  // "disconnected", "connecting", etc.
```

## Type Conversion Reference

When using generated C++ code:

```cpp
// IDL: string name
msg.name("value");          // Set
std::string n = msg.name(); // Get

// IDL: long count
msg.count(42);              // Set
int32_t c = msg.count();    // Get

// IDL: sequence<double> values
std::vector<double> vals = {1.0, 2.0};
msg.values(vals);                        // Set
std::vector<double> v = msg.values();    // Get

// IDL: Position position (nested struct)
msg.position().x(10.0);     // Set field
double x = msg.position().x(); // Get field
```

## Summary

Key points:
- Simple syntax: `struct Name { fields; };`
- Rich type system: primitives, strings, arrays, sequences
- Supports enums, unions, nested structs, maps
- Annotations for advanced features
- Always add ID and timestamp fields
- Use meaningful names and comments

For more details, see [Adding New Message Types](ADD_NEW_TYPE.md).
