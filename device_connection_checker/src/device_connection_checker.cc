/*
 * Copyright 2016-2025. UISEE TECHNOLOGIES (BEIJING) LTD. All rights reserved.
 * See LICENSE AGREEMENT file in the project root for full license information.
 */

#include "../include/device_connection_checker.h"
#include "../include/device_connection_checker_defaults.h"
#include <sstream>

const char* DeviceConnectionChecker::LOG_TAG = "DeviceChecker";

DeviceConnectionChecker::DeviceConnectionChecker(std::shared_ptr<ILogger> logger,
                                                 std::shared_ptr<ITimeProvider> time_provider)
    : mLogger(logger ? logger : std::make_shared<ConsoleLogger>()),
      mTimeProvider(time_provider ? time_provider : std::make_shared<SystemTimeProvider>()),
      mStatusCallback(nullptr)
{
}

DeviceConnectionChecker::~DeviceConnectionChecker()
{
    // Clean up all devices
    for (auto &pair : mDeviceStates)
    {
        pair.second.cleanup_device();
    }
    mDeviceStates.clear();
}

// Log helper methods
void DeviceConnectionChecker::log_error(const std::string& message)
{
    if (mLogger) {
        mLogger->log_error(LOG_TAG, message);
    }
}

void DeviceConnectionChecker::log_warning(const std::string& message)
{
    if (mLogger) {
        mLogger->log_warning(LOG_TAG, message);
    }
}

void DeviceConnectionChecker::log_info(const std::string& message)
{
    if (mLogger) {
        mLogger->log_info(LOG_TAG, message);
    }
}

void DeviceConnectionChecker::set_status_callback(device_status_callback_t callback)
{
    mStatusCallback = callback;
}

bool_t DeviceConnectionChecker::register_device(std::shared_ptr<IConnectableDevice> device,
                                                const CheckConfiguration& config)
{
    if (device == nullptr)
    {
        log_error("Device pointer is null");
        return UOS_FALSE;
    }

    std::string device_name = device->get_device_name();
    if (device_name.empty())
    {
        log_error("Device name is empty");
        return UOS_FALSE;
    }

    if (mDeviceStates.find(device_name) != mDeviceStates.end())
    {
        std::ostringstream oss;
        oss << "Device " << device_name << " already registered";
        log_warning(oss.str());
        return UOS_FALSE;
    }

    DeviceState state;
    state.device = device;
    state.config = config;  // Store configuration
    state.status = DEVICE_STATUS_UNKNOWN;

    // Initialize device with its own communication resources
    if (state.initialize_device() == UOS_FALSE)
    {
        std::ostringstream oss;
        oss << "Failed to initialize device: " << device_name;
        log_error(oss.str());
        return UOS_FALSE;
    }

    mDeviceStates[device_name] = state;

    std::ostringstream oss;
    oss << "Registered device: " << device_name
        << " (timeout=" << config.check_timeout
        << "s, retry=" << config.max_retry_count << ")";
    log_info(oss.str());

    return UOS_TRUE;
}

bool_t DeviceConnectionChecker::start_check(const std::string &device_name)
{
    auto state_it = mDeviceStates.find(device_name);
    if (state_it == mDeviceStates.end())
    {
        std::ostringstream oss;
        oss << "Device " << device_name << " not found";
        log_error(oss.str());
        return UOS_FALSE;
    }

    DeviceState &state = state_it->second;
    state.status = DEVICE_STATUS_CHECKING;
    state.retry_count = 0;
    state.last_ping_ts = 0.0;
    state.last_pong_ts = 0.0;
    return send_ping(state);
}

bool_t DeviceConnectionChecker::unregister_device(const std::string &device_name)
{
    if (device_name.empty())
    {
        log_error("Device name is empty");
        return UOS_FALSE;
    }

    auto state_it = mDeviceStates.find(device_name);
    if (state_it == mDeviceStates.end())
    {
        std::ostringstream oss;
        oss << "Device " << device_name << " not found";
        log_warning(oss.str());
        return UOS_FALSE;
    }

    state_it->second.cleanup_device();
    mDeviceStates.erase(state_it);

    std::ostringstream oss;
    oss << "Unregistered device: " << device_name;
    log_info(oss.str());
    return UOS_TRUE;
}

bool_t DeviceConnectionChecker::start_check_all()
{
    bool_t ret = UOS_TRUE;

    for (auto &pair : mDeviceStates)
    {
        if (UOS_FALSE == start_check(pair.first))
        {
            ret = UOS_FALSE;
        }
    }

    return ret;
}

bool_t DeviceConnectionChecker::send_ping(DeviceState &dev_state)
{
    if (dev_state.device.get() == nullptr)
    {
        log_error("Device pointer is null");
        return UOS_FALSE;
    }

    // Use device's own send_ping implementation (LOD compliant)
    bool_t ret = dev_state.send_device_ping();

    if (ret == UOS_TRUE)
    {
        dev_state.last_ping_ts = mTimeProvider->get_current_time();

        std::ostringstream oss;
        oss << "Sent ping to device: " << dev_state.get_device_name();
        log_info(oss.str());
    }
    else
    {
        std::ostringstream oss;
        oss << "Failed to send ping to device: " << dev_state.get_device_name();
        log_error(oss.str());
    }

    return ret;
}

bool_t DeviceConnectionChecker::check_timeout(DeviceState &dev_state)
{
    if (dev_state.status != DEVICE_STATUS_CHECKING || dev_state.device == nullptr)
    {
        return UOS_FALSE;
    }

    float64_t cts = mTimeProvider->get_current_time();
    float64_t timeout = dev_state.get_check_timeout();  // LOD compliant
    int32_t max_retry = dev_state.get_max_retry_count(); // LOD compliant

    // Check if timeout occurred
    if (uos_fgreater(cts - dev_state.last_ping_ts, timeout))
    {
        dev_state.retry_count++;

        if (dev_state.retry_count >= max_retry)
        {
            // Max retry count reached, mark as offline
            update_device_status(dev_state, DEVICE_STATUS_OFFLINE);

            std::ostringstream oss;
            oss << "Device " << dev_state.get_device_name()
                << " is OFFLINE (no response after " << max_retry << " retries)";
            log_warning(oss.str());

            return UOS_TRUE;
        }
        else
        {
            // Retry
            std::ostringstream oss;
            oss << "Device " << dev_state.get_device_name()
                << " check timeout, retry " << dev_state.retry_count << "/" << max_retry;
            log_info(oss.str());

            send_ping(dev_state);
        }
    }

    return UOS_FALSE;
}

bool_t DeviceConnectionChecker::update_device_status(DeviceState &dev_state,
                                                      device_connection_status_t new_status)
{
    if (dev_state.status == new_status || dev_state.device == nullptr)
    {
        return UOS_FALSE;
    }

    device_connection_status_t old_status = dev_state.status;
    dev_state.status = new_status;

    std::string device_name = dev_state.get_device_name(); // LOD compliant

    // Call device-specific callback (LOD compliant)
    dev_state.notify_status_changed(old_status, new_status);

    // Call global callback if set
    if (mStatusCallback)
    {
        mStatusCallback(device_name, old_status, new_status);
    }

    return UOS_TRUE;
}

bool_t DeviceConnectionChecker::update()
{
    for (auto &pair : mDeviceStates)
    {
        DeviceState &dev_state = pair.second;

        if (dev_state.status == DEVICE_STATUS_CHECKING && dev_state.device != nullptr)
        {
            // LOD compliant - use encapsulated method
            if (dev_state.is_device_responding() == UOS_TRUE)
            {
                dev_state.last_pong_ts = mTimeProvider->get_current_time();
                update_device_status(dev_state, DEVICE_STATUS_ONLINE);

                float64_t response_time_ms = (dev_state.last_pong_ts - dev_state.last_ping_ts) * 1000.0;
                std::ostringstream oss;
                oss << "Device " << dev_state.get_device_name()
                    << " is ONLINE (response time: " << response_time_ms << " ms)";
                log_info(oss.str());

                continue;
            }
        }

        check_timeout(dev_state);
    }

    return UOS_TRUE;
}

device_connection_status_t DeviceConnectionChecker::get_device_status(const std::string &device_name) const
{
    auto state_it = mDeviceStates.find(device_name);
    if (state_it != mDeviceStates.end())
    {
        return state_it->second.status;
    }

    return DEVICE_STATUS_UNKNOWN;
}

