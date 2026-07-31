/*
 * Copyright 2016-2025. UISEE TECHNOLOGIES (BEIJING) LTD. All rights reserved.
 * See LICENSE AGREEMENT file in the project root for full license information.
 */
#ifndef __DEVICE_CONNECTION_CHECKER_H__
#define __DEVICE_CONNECTION_CHECKER_H__

#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <memory>
#include "device_connection_checker_types.h"
#include "device_connection_checker_interfaces.h"

/**
 * Device status change callback
 * Parameters: device_name, old_status, new_status
 */
typedef std::function<void(const std::string&, device_connection_status_t, device_connection_status_t)>
    device_status_callback_t;



/**
 * Device connection checker - manages device connection checking
 * Follows Dependency Inversion Principle by depending on abstractions
 */
class DeviceConnectionChecker
{
public:
    /**
     * Constructor with dependency injection
     * @param logger Logger implementation (optional, uses default if nullptr)
     * @param time_provider Time provider implementation (optional, uses default if nullptr)
     */
    explicit DeviceConnectionChecker(std::shared_ptr<ILogger> logger = nullptr,
                                    std::shared_ptr<ITimeProvider> time_provider = nullptr);
    ~DeviceConnectionChecker();

    /**
     * Register a device to check
     * @param device Device instance that implements IConnectableDevice
     * @param config Check configuration (timeout, retry settings)
     * @return UOS_TRUE if registered successfully
     */
    bool_t register_device(std::shared_ptr<IConnectableDevice> device,
                          const CheckConfiguration& config = CheckConfiguration());

    /**
     * Unregister a device
     * @param device_name Device name
     * @return UOS_TRUE if unregistered successfully
     */
    bool_t unregister_device(const std::string &device_name);

    /**
     * Start checking a specific device
     */
    bool_t start_check(const std::string &device_name);

    /**
     * Start checking all devices
     */
    bool_t start_check_all();

    /**
     * Periodic update (call in main loop)
     */
    bool_t update();
    
    /**
     * Get device status
     */
    device_connection_status_t get_device_status(const std::string &device_name) const;

    /**
     * Set global status change callback
     * @param callback Callback function to be called when any device status changes
     */
    void set_status_callback(device_status_callback_t callback);

private:
    /**
     * Internal device state tracking
     * Encapsulates device access to follow Law of Demeter
     */
    class DeviceState {
    public:
        std::shared_ptr<IConnectableDevice> device; // Device instance
        CheckConfiguration config;                  // Check configuration
        device_connection_status_t status;          // Current status
        float64_t last_ping_ts;                     // Last ping timestamp
        float64_t last_pong_ts;                     // Last pong timestamp
        int32_t retry_count;                        // Current retry count

        DeviceState() : device(nullptr), status(DEVICE_STATUS_UNKNOWN),
                       last_ping_ts(0.0), last_pong_ts(0.0),
                       retry_count(0) {}

        // Encapsulation methods to follow Law of Demeter
        const char* get_device_name_cstr() const {
            return device ? device->get_device_name().c_str() : "unknown";
        }

        std::string get_device_name() const {
            return device ? device->get_device_name() : "unknown";
        }

        // Configuration accessors - now from local config, not device
        float64_t get_check_timeout() const {
            return config.check_timeout;
        }

        float64_t get_retry_interval() const {
            return config.retry_interval;
        }

        int32_t get_max_retry_count() const {
            return config.max_retry_count;
        }

        bool_t is_device_responding() const {
            return device ? device->is_responding() : UOS_FALSE;
        }

        bool_t send_device_ping() {
            return device ? device->send_ping() : UOS_FALSE;
        }

        bool_t initialize_device() {
            return device ? device->initialize() : UOS_FALSE;
        }

        bool_t cleanup_device() {
            return device ? device->cleanup() : UOS_FALSE;
        }

        void notify_status_changed(device_connection_status_t old_status,
                                   device_connection_status_t new_status) {
            if (device) {
                device->on_status_changed(old_status, new_status);
            }
        }
    };

    /**
     * Send ping for a device
     */
    bool_t send_ping(DeviceState &dev_state);

    /**
     * Check timeout for a device
     */
    bool_t check_timeout(DeviceState &dev_state);

    /**
     * Update device status
     */
    bool_t update_device_status(DeviceState &dev_state, device_connection_status_t new_status);

    /**
     * Log helper methods
     */
    void log_error(const std::string& message);
    void log_warning(const std::string& message);
    void log_info(const std::string& message);

private:
    std::map<std::string, DeviceState> mDeviceStates;        // Device state tracking
    std::shared_ptr<ILogger> mLogger;                        // Logger dependency
    std::shared_ptr<ITimeProvider> mTimeProvider;            // Time provider dependency
    device_status_callback_t mStatusCallback;                // Global status callback

    static const char* LOG_TAG;                              // Log tag constant
};

#endif /* __DEVICE_CONNECTION_CHECKER_H__ */

