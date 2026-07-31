/*
 * Copyright 2016-2025. UISEE TECHNOLOGIES (BEIJING) LTD. All rights reserved.
 * See LICENSE AGREEMENT file in the project root for full license information.
 */
#ifndef __DEVICE_CONNECTION_CHECKER_INTERFACES_H__
#define __DEVICE_CONNECTION_CHECKER_INTERFACES_H__

#include <string>
#include "device_connection_checker_types.h"

/**
 * Logger interface - abstracts logging functionality
 * Follows Dependency Inversion Principle
 */
class ILogger
{
public:
    virtual ~ILogger() {}
    
    virtual void log_error(const std::string& tag, const std::string& message) = 0;
    virtual void log_warning(const std::string& tag, const std::string& message) = 0;
    virtual void log_info(const std::string& tag, const std::string& message) = 0;
};

/**
 * Time provider interface - abstracts time functionality
 * Follows Dependency Inversion Principle
 */
class ITimeProvider
{
public:
    virtual ~ITimeProvider() {}
    
    virtual float64_t get_current_time() const = 0;
};

/**
 * Device identity interface
 * Follows Interface Segregation Principle
 */
class IDevice
{
public:
    virtual ~IDevice() {}
    
    /**
     * Get device name
     */
    virtual std::string get_device_name() const = 0;
};

/**
 * Device lifecycle interface
 * Follows Interface Segregation Principle
 */
class ILifecycle
{
public:
    virtual ~ILifecycle() {}
    
    /**
     * Initialize device internal resources
     * @return UOS_TRUE if initialization successful
     */
    virtual bool_t initialize() = 0;
    
    /**
     * Cleanup device resources
     * @return UOS_TRUE if cleanup successful
     */
    virtual bool_t cleanup() = 0;
};

/**
 * Pingable device interface
 * Follows Interface Segregation Principle
 */
class IPingable
{
public:
    virtual ~IPingable() {}
    
    /**
     * Send ping message to device
     * @return UOS_TRUE if ping sent successfully
     */
    virtual bool_t send_ping() = 0;
    
    /**
     * Check if device is responding
     * @return UOS_TRUE if device responded (pong received)
     */
    virtual bool_t is_responding() const = 0;
};

/**
 * Device connection status
 */
typedef enum _Device_Connection_Status
{
    DEVICE_STATUS_UNKNOWN = 0,      // Unknown status
    DEVICE_STATUS_CHECKING,         // Checking in progress
    DEVICE_STATUS_ONLINE,           // Device is online
    DEVICE_STATUS_OFFLINE,          // Device is offline
    DEVICE_STATUS_TIMEOUT           // Check timeout
} device_connection_status_t;

/**
 * Status observer interface
 * Follows Interface Segregation Principle
 */
class IStatusObserver
{
public:
    virtual ~IStatusObserver() {}

    /**
     * Called when device status changes
     */
    virtual void on_status_changed(device_connection_status_t old_status,
                                   device_connection_status_t new_status) {}
};

/**
 * Complete connectable device interface
 * Composes all required interfaces
 * Note: Check configuration (timeout, retry) is managed by DeviceConnectionChecker,
 *       not by individual devices. This follows Single Responsibility Principle.
 */
class IConnectableDevice : public IDevice,
                          public ILifecycle,
                          public IPingable,
                          public IStatusObserver
{
public:
    virtual ~IConnectableDevice() {}
};

/**
 * Check configuration structure
 * Managed by DeviceConnectionChecker for each device
 */
struct CheckConfiguration
{
    float64_t check_timeout;      // Check timeout in seconds (default: 2.0)
    float64_t retry_interval;     // Retry interval in seconds (default: 3.0)
    int32_t max_retry_count;      // Maximum retry count (default: 3)

    CheckConfiguration()
        : check_timeout(2.0),
          retry_interval(3.0),
          max_retry_count(3) {}

    CheckConfiguration(float64_t timeout, float64_t interval, int32_t max_retry)
        : check_timeout(timeout),
          retry_interval(interval),
          max_retry_count(max_retry) {}
};

#endif /* __DEVICE_CONNECTION_CHECKER_INTERFACES_H__ */

