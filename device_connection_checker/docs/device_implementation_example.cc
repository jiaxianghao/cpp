/*
 * IConnectableDevice implementation examples (interface-based design).
 *
 * Each device:
 *  - Uses its own CAN sender (ICanInterface) to transmit ping frames
 *  - Registers receive callbacks during initialize() via a registrar
 *  - Validates pong payloads internally and toggles its responding flag
 *  - Notifies DeviceConnectionChecker only through status changes
 *
 * Replace the registrar with your actual CAN driver hooks. The code below
 * focuses on encapsulation patterns rather than specific driver APIs.
 */

#include "../include/device_connection_checker.h"

#include <functional>
#include <map>
#include <string>
#include <uos_base/uos_base.h>

using std::function;
using std::map;
using std::string;

class ICanInterface
{
public:
    virtual ~ICanInterface() {}
    virtual bool_t send_can_message(uint16_t can_id, uint8_t *data, int32_t len) = 0;
};

class CanCallbackRegistrar
{
public:
    using Handler = function<void(uint16_t, uint8_t*, int32_t)>;

    void register_rx_handler(const string &name, Handler handler)
    {
        mHandlers[name] = handler;
    }

    void remove_rx_handler(const string &name)
    {
        mHandlers.erase(name);
    }

    void dispatch(uint16_t can_id, uint8_t *data, int32_t len)
    {
        for (auto &entry : mHandlers)
        {
            entry.second(can_id, data, len);
        }
    }

private:
    map<string, Handler> mHandlers;
};

class AutohookDevice : public IConnectableDevice
{
public:
    AutohookDevice(ICanInterface *can, CanCallbackRegistrar *registrar)
        : mCan(can), mRegistrar(registrar), mIsResponding(UOS_FALSE) {}

    string get_device_name() const override
    {
        return "autohook";
    }

    bool_t initialize() override
    {
        if (mCan == nullptr || mRegistrar == nullptr)
        {
            return UOS_FALSE;
        }

        mRegistrar->register_rx_handler(get_device_name(),
            [this](uint16_t can_id, uint8_t *data, int32_t len) {
                this->on_can_message(can_id, data, len);
            });
        return UOS_TRUE;
    }

    bool_t cleanup() override
    {
        if (mRegistrar != nullptr)
        {
            mRegistrar->remove_rx_handler(get_device_name());
        }
        mIsResponding = UOS_FALSE;
        return UOS_TRUE;
    }

    bool_t send_ping() override
    {
        uint8_t ping_data[8] = {0xAA, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55};
        mIsResponding = UOS_FALSE;
        return mCan->send_can_message(0x100, ping_data, 8);
    }

    bool_t is_responding() const override
    {
        return mIsResponding;
    }

    void on_status_changed(device_connection_status_t old_status,
                           device_connection_status_t new_status) override
    {
        uos_log_i(UOS_MOD_OPERATION, "[Autohook] %d -> %d", old_status, new_status);
    }

private:
    void on_can_message(uint16_t can_id, uint8_t *data, int32_t len)
    {
        if (can_id == 0x101 && len >= 1 && data[0] == 0xBB)
        {
            mIsResponding = UOS_TRUE;
        }
    }

    ICanInterface *mCan;
    CanCallbackRegistrar *mRegistrar;
    bool_t mIsResponding;
};

class LEDDisplayDevice : public IConnectableDevice
{
public:
    LEDDisplayDevice(ICanInterface *can, CanCallbackRegistrar *registrar)
        : mCan(can), mRegistrar(registrar), mIsResponding(UOS_FALSE) {}

    string get_device_name() const override
    {
        return "led_display";
    }

    bool_t initialize() override
    {
        if (mCan == nullptr || mRegistrar == nullptr)
        {
            return UOS_FALSE;
        }

        mRegistrar->register_rx_handler(get_device_name(),
            [this](uint16_t can_id, uint8_t *data, int32_t len) {
                this->on_can_message(can_id, data, len);
            });
        return UOS_TRUE;
    }

    bool_t cleanup() override
    {
        if (mRegistrar != nullptr)
        {
            mRegistrar->remove_rx_handler(get_device_name());
        }
        mIsResponding = UOS_FALSE;
        return UOS_TRUE;
    }

    bool_t send_ping() override
    {
        uint8_t ping_data[8] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        mIsResponding = UOS_FALSE;
        return mCan->send_can_message(0x200, ping_data, 8);
    }

    bool_t is_responding() const override
    {
        return mIsResponding;
    }

private:
    void on_can_message(uint16_t can_id, uint8_t *data, int32_t len)
    {
        if (can_id == 0x201 && len >= 1 && data[0] == 0x02)
        {
            mIsResponding = UOS_TRUE;
        }
    }

    ICanInterface *mCan;
    CanCallbackRegistrar *mRegistrar;
    bool_t mIsResponding;
};

class AVASDevice : public IConnectableDevice
{
public:
    AVASDevice(ICanInterface *can, CanCallbackRegistrar *registrar)
        : mCan(can), mRegistrar(registrar), mIsResponding(UOS_FALSE) {}

    string get_device_name() const override
    {
        return "avas";
    }

    bool_t initialize() override
    {
        if (mCan == nullptr || mRegistrar == nullptr)
        {
            return UOS_FALSE;
        }

        mRegistrar->register_rx_handler(get_device_name(),
            [this](uint16_t can_id, uint8_t *data, int32_t len) {
                this->on_can_message(can_id, data, len);
            });
        return UOS_TRUE;
    }

    bool_t cleanup() override
    {
        if (mRegistrar != nullptr)
        {
            mRegistrar->remove_rx_handler(get_device_name());
        }
        mIsResponding = UOS_FALSE;
        return UOS_TRUE;
    }

    bool_t send_ping() override
    {
        uint8_t ping_data[8] = {0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        mIsResponding = UOS_FALSE;
        return mCan->send_can_message(0x300, ping_data, 8);
    }

    bool_t is_responding() const override
    {
        return mIsResponding;
    }

private:
    void on_can_message(uint16_t can_id, uint8_t *data, int32_t len)
    {
        if (can_id == 0x301 && len >= 1 && data[0] == 0xFE)
        {
            mIsResponding = UOS_TRUE;
        }
    }

    ICanInterface *mCan;
    CanCallbackRegistrar *mRegistrar;
    bool_t mIsResponding;
};
