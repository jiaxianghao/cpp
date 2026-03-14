# Adding New Message Types

This guide shows you how to add new message types to your DDS application with zero modifications to the wrapper code.

## Overview

The 3-step process:

1. **Define IDL** - Create your message structure
2. **Build** - CMake auto-generates C++ code
3. **Use** - Templates work automatically

## Step-by-Step Tutorial

### Step 1: Define Your IDL File

Create a new file in the `idl/` directory. For example, `idl/RobotStatus.idl`:

```idl
struct RobotStatus
{
    string robot_id;
    double position_x;
    double position_y;
    double position_z;
    long battery_level;
    long timestamp;
};
```

### Step 2: Rebuild the Project

```bash
cd build
cmake ..
make
```

CMake automatically:
- Detects the new IDL file
- Runs fastddsgen to generate C++ code
- Compiles and links it to the library

### Step 3: Use in Your Code

```cpp
#include "dds_wrapper/DDSManager.h"
#include "RobotStatus.h"  // Auto-generated header

using namespace dds_wrapper;

int main()
{
    DDSManager manager;
    manager.initialize();

    // Create publisher - templates handle everything!
    auto pub = manager.createPublisher<RobotStatus>("RobotTopic");

    // Create and send message
    RobotStatus status;
    status.robot_id("ROBOT_001");
    status.position_x(10.5);
    status.position_y(20.3);
    status.position_z(0.0);
    status.battery_level(85);
    status.timestamp(std::chrono::system_clock::now().time_since_epoch().count());

    pub->publish(status);

    // Create subscriber
    auto sub = manager.createSubscriber<RobotStatus>("RobotTopic",
        [](const RobotStatus& msg)
        {
            std::cout << "Robot " << msg.robot_id() 
                      << " at (" << msg.position_x() 
                      << ", " << msg.position_y() << ")" << std::endl;
        });

    return 0;
}
```

That's it! No wrapper code modification needed.

## Using the IDL Generator Tool

For quick IDL file creation, use the interactive tool:

```bash
cd tools
python create_idl.py --interactive
```

Follow the prompts to create:
- Simple structs
- Sensor data messages
- Command messages
- Complex nested structures

### Command-Line Usage

```bash
# Create a simple struct
python create_idl.py --name MyMessage --type simple --output ../idl

# Create a sensor data message
python create_idl.py --name TemperatureSensor --type sensor --output ../idl

# Create a command message
python create_idl.py --name RobotCommand --type command --output ../idl
```

## IDL Syntax Reference

### Basic Types

```idl
struct BasicTypes
{
    boolean flag;
    char character;
    octet byte_value;
    short int16_value;
    long int32_value;
    long long int64_value;
    float float32_value;
    double float64_value;
    string text;
};
```

### Arrays and Sequences

```idl
struct Collections
{
    // Fixed-size array
    double coordinates[3];
    
    // Variable-length sequence
    sequence<double> values;
    
    // Bounded sequence (max 100 elements)
    sequence<double, 100> bounded_values;
    
    // String sequences
    sequence<string> names;
};
```

### Enumerations

```idl
enum Status
{
    IDLE,
    RUNNING,
    PAUSED,
    ERROR
};

struct DeviceState
{
    string device_id;
    Status current_status;
};
```

### Nested Structures

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

### Complex Example

```idl
enum VehicleType
{
    CAR,
    TRUCK,
    BUS,
    MOTORCYCLE
};

struct GPS
{
    double latitude;
    double longitude;
    double altitude;
};

struct VehicleData
{
    string vehicle_id;
    VehicleType type;
    GPS location;
    sequence<GPS> route;
    double speed;
    long timestamp;
};
```

## Validating IDL Files

Before building, validate your IDL syntax:

```bash
cd tools
./validate_idl.sh ../idl
```

This checks all IDL files for syntax errors.

## Listing Available Types

See all registered message types:

```bash
./build/bin/tools/list_types
```

Output:
```
Available DDS Message Types
==========================================

1. CommonMessage
   Description: General purpose message
   IDL File:    idl/CommonMessage.idl

2. SensorData
   Description: Sensor data message
   IDL File:    idl/SensorData.idl

...
```

## Best Practices

### 1. Use Meaningful Names

```idl
// Good
struct TemperatureSensorReading { ... }

// Avoid
struct Data1 { ... }
```

### 2. Include Timestamps

```idl
struct MyMessage
{
    string id;
    // ... other fields ...
    long timestamp;  // Always useful for debugging
};
```

### 3. Add Comments

```idl
// Temperature sensor reading in Celsius
struct TemperatureReading
{
    string sensor_id;      // Unique sensor identifier
    double temperature;    // Temperature in Celsius
    boolean is_valid;      // Data validity flag
    long timestamp;        // Unix timestamp in milliseconds
};
```

### 4. Version Your Messages

```idl
struct SensorDataV2  // Version in name
{
    string sensor_id;
    long version;    // Or version field
    // ... fields ...
};
```

## Advanced Topics

### Using Maps

```idl
struct ConfigMessage
{
    string device_id;
    map<string, string> parameters;
    long timestamp;
};
```

### Optional Fields

```idl
struct ExtendedData
{
    string id;
    @optional string description;
    @optional double optional_value;
};
```

### Unions

```idl
union SensorValue switch(long)
{
    case 1: double temperature;
    case 2: long pressure;
    case 3: boolean status;
};

struct GenericSensor
{
    string sensor_id;
    SensorValue value;
};
```

## Troubleshooting

### IDL File Not Detected

- Ensure file is in `idl/` directory
- Check file extension is `.idl`
- Re-run CMake: `cd build && cmake ..`

### Compilation Errors

- Validate IDL syntax: `./tools/validate_idl.sh idl`
- Check FastDDS-Gen version compatibility
- Review FastDDS documentation

### Type Not Found

- Verify IDL was successfully compiled
- Check if header file was generated in `build/idl/`
- Include the generated header: `#include "YourType.h"`

## Summary

Adding new types is simple:

```bash
# 1. Create IDL
echo 'struct MyType { string id; long value; };' > idl/MyType.idl

# 2. Build
cd build && cmake .. && make

# 3. Use
# auto pub = manager.createPublisher<MyType>("Topic");
```

No wrapper modification required! 🎉
