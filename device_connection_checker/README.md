# Device Connection Checker

A generic tool for detecting external device connections via CAN bus communication.

## Overview

The Device Connection Checker provides a reliable way to verify that external hardware devices (such as autohook, LED displays, AVAS, etc.) are properly connected by sending ping messages and waiting for pong responses.

**NEW**: Now supports interface-based design where each device implements its own ping/pong protocol!

## Features

- ✅ **Interface-Based Design** (NEW): Each device implements `IConnectableDevice` for custom protocols
- ✅ **Active Detection**: Send ping messages to devices and wait for responses
- ✅ **Flexible Matching**: Support data masking for partial byte matching (legacy mode)
- ✅ **Reliable Retry**: Configurable timeout and retry mechanism
- ✅ **Status Management**: Track device online/offline status
- ✅ **Callback Support**: Get notified when device status changes
- ✅ **CAN Agnostic**: Works with any CAN interface implementation
- ✅ **Backward Compatible**: Still supports legacy config-based mode

## Directory Structure

```
device_connection_checker/
├── include/
│   └── device_connection_checker.h    # Header file
├── src/
│   └── device_connection_checker.cc   # Implementation
├── etc/
│   └── device_connection_check.json   # Configuration example
├── docs/
│   ├── README.md                      # This file
│   ├── API.md                         # API documentation
│   ├── INTEGRATION.md                 # Integration guide
│   └── example_usage.cc               # Usage example
└── CMakeLists.txt                     # Build configuration
```

## Quick Start

### Quick Start (Interface-Based) ⭐

#### 1. Implement Your Device Class

```cpp
#include "device_connection_checker.h"

class AutohookDevice : public IConnectableDevice
{
public:
    explicit AutohookDevice(MyCanInterface *can)
        : mCan(can), mIsResponding(UOS_FALSE) {}

    string get_device_name() const override {
        return "autohook";
    }

    bool_t initialize() override {
        if (mCan == nullptr) {
            return UOS_FALSE;
        }

        // Register CAN receive callback with your driver
        mCan->set_receive_callback("autohook",
            [this](uint16_t can_id, uint8_t *data, int32_t len) {
                this->on_can_message(can_id, data, len);
            });
        return UOS_TRUE;
    }

    bool_t cleanup() override {
        if (mCan != nullptr) {
            mCan->remove_receive_callback("autohook");
        }
        mIsResponding = UOS_FALSE;
        return UOS_TRUE;
    }

    bool_t send_ping() override {
        uint8_t ping_data[8] = {0xAA, 0, 0, 0, 0, 0, 0, 0};
        mIsResponding = UOS_FALSE;
        return mCan->send_can_message(0x100, ping_data, 8);
    }

    bool_t is_responding() const override {
        return mIsResponding;
    }

    void on_status_changed(device_connection_status_t old_status,
                           device_connection_status_t new_status) override {
        if (new_status == DEVICE_STATUS_ONLINE) {
            // Handle device online
        }
    }

private:
    void on_can_message(uint16_t can_id, uint8_t *data, int32_t len) {
        if (can_id == 0x101 && len >= 1 && data[0] == 0xBB) {
            mIsResponding = UOS_TRUE;
        }
    }

    MyCanInterface *mCan;
    bool_t mIsResponding;
};
```

#### 2. Register and Use

```cpp
DeviceConnectionChecker checker;
MyCanInterface can_interface;
auto autohook = std::make_shared<AutohookDevice>(&can_interface);

checker.register_device(autohook);
checker.start_check("autohook");
```

### Main Loop

```cpp
while (running) {
    checker.update();  // Check timeouts and retries
    
    // Check device status
    auto status = checker.get_device_status("autohook");
    if (status == DEVICE_STATUS_ONLINE) {
        // Device is ready
    }
    
    usleep(100000);  // 100ms
}
```

## Configuration

### Device Parameters

| Parameter | Type | Description | Recommended Value |
|-----------|------|-------------|-------------------|
| `device_name` | string | Device identifier | "autohook", "led", etc. |
| `ping_can_id` | uint16_t | Ping message CAN ID | Device specific |
| `pong_can_id` | uint16_t | Pong response CAN ID | Device specific |
| `ping_data[8]` | uint8_t[] | Ping message data | Device protocol |
| `expected_pong_data[8]` | uint8_t[] | Expected response | Device protocol |
| `pong_data_mask[8]` | uint8_t[] | Byte matching mask | 0xFF=match, 0x00=ignore |
| `check_timeout` | float64_t | Timeout in seconds | 1.5 - 3.0 |
| `retry_interval` | float64_t | Retry interval | 2.0 - 5.0 |
| `max_retry_count` | int32_t | Maximum retries | 2 - 5 |

### Data Mask Usage

The `pong_data_mask` allows flexible matching:

```cpp
// Match all bytes
pong_data_mask = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Match only first 2 bytes (ignore dynamic data like counters)
pong_data_mask = {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Match only first byte
pong_data_mask = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
```

## Device Status

| Status | Description |
|--------|-------------|
| `DEVICE_STATUS_UNKNOWN` | Initial state, not checked yet |
| `DEVICE_STATUS_CHECKING` | Check in progress |
| `DEVICE_STATUS_ONLINE` | Device responded correctly |
| `DEVICE_STATUS_OFFLINE` | No response after max retries |
| `DEVICE_STATUS_TIMEOUT` | Check timeout |

## Workflow

```
┌─────────────┐
│ Register    │
│ Device      │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│ Start       │
│ Check       │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│ Send Ping   │
└──────┬──────┘
       │
       ▼
┌─────────────────────┐
│ Wait for Pong       │
│ (timeout: 1.5-3.0s) │
└──────┬──────────────┘
       │
       ├─────────────┐
       │             │
       ▼             ▼
┌──────────┐   ┌──────────┐
│ Received │   │ Timeout  │
└────┬─────┘   └────┬─────┘
     │              │
     ▼              ▼
┌──────────┐   ┌──────────┐
│ Verify   │   │ Retry?   │
│ Data     │   │          │
└────┬─────┘   └────┬─────┘
     │              │
     ├──────────────┤
     │              │
     ▼              ▼
┌──────────┐   ┌──────────┐
│ ONLINE   │   │ OFFLINE  │
└──────────┘   └──────────┘
```

## Examples

See `docs/example_usage.cc` for complete examples including:
- Autohook device detection
- LED display detection
- AVAS device detection
- Status change callbacks
- Main loop integration

## Integration

For a complete runnable example, see `docs/example_usage.cc`.

## API Reference

For device implementation patterns, see `docs/device_implementation_example.cc`.

## Building

```bash
mkdir build && cd build
cmake ..
make
```

### Build Options

- `BUILD_DEVICE_CHECKER_EXAMPLE`: Build example program (default: OFF)
- `BUILD_DEVICE_CHECKER_TESTS`: Build unit tests (default: ON)

```bash
# Build with tests
cmake -DBUILD_DEVICE_CHECKER_TESTS=ON ..
make

# Build with example
cmake -DBUILD_DEVICE_CHECKER_EXAMPLE=ON ..
make
```

## Testing

### Run Unit Tests

```bash
# Build and run tests
mkdir build && cd build
cmake -DBUILD_DEVICE_CHECKER_TESTS=ON ..
make
./device_connection_checker_ut

# Or use CTest
ctest --verbose
```

### Test Results

All 13 unit tests pass successfully ✅

### Test Coverage

The unit tests cover:
- ✅ Device registration (normal, empty name, duplicate)
- ✅ Start check (send ping, non-existent device)
- ✅ Pong message handling (online, data mismatch)
- ✅ Data mask (full match, partial match)
- ✅ Status callbacks (device-specific, global)
- ✅ Multiple devices (start check all)
- ✅ Error handling (CAN interface not set, non-existent device)

### Run Example

```bash
# Build example
cmake -DBUILD_DEVICE_CHECKER_EXAMPLE=ON ..
make
./device_connection_checker_example
```

## License

Copyright 2016-2025. UISEE TECHNOLOGIES (BEIJING) LTD. All rights reserved.
