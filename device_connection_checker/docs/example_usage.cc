/*
 * Device Connection Checker - Usage Example (Interface-Based)
 *
 * This example demonstrates how each device encapsulates its own
 * communication logic while DeviceConnectionChecker manages state.
 */

#include "../include/device_connection_checker.h"

#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unistd.h>

using std::function;
using std::make_shared;
using std::map;
using std::shared_ptr;
using std::string;

class SimulatedCanBus
{
public:
    using RxHandler = function<void(uint16_t, uint8_t*, int32_t)>;

    bool_t send(uint16_t can_id, uint8_t *data, int32_t len)
    {
        printf("[CAN] TX 0x%03X :", can_id);
        for (int32_t i = 0; i < len; ++i)
        {
            printf(" %02X", data[i]);
        }
        printf("\n");
        return UOS_TRUE;
    }

    void register_handler(const string &name, RxHandler handler)
    {
        mHandlers[name] = handler;
    }

    void unregister_handler(const string &name)
    {
        mHandlers.erase(name);
    }

    void simulate_receive(uint16_t can_id, uint8_t *data, int32_t len)
    {
        for (auto &entry : mHandlers)
        {
            entry.second(can_id, data, len);
        }
    }

private:
    map<string, RxHandler> mHandlers;
};

class AutohookDevice : public IConnectableDevice
{
public:
    explicit AutohookDevice(SimulatedCanBus *bus)
        : mBus(bus), mIsResponding(UOS_FALSE) {}

    string get_device_name() const override
    {
        return "autohook";
    }

    bool_t initialize() override
    {
        if (mBus == nullptr)
        {
            return UOS_FALSE;
        }

        mBus->register_handler(get_device_name(),
            [this](uint16_t can_id, uint8_t *data, int32_t len) {
                this->on_can_message(can_id, data, len);
            });
        return UOS_TRUE;
    }

    bool_t cleanup() override
    {
        if (mBus != nullptr)
        {
            mBus->unregister_handler(get_device_name());
        }
        mIsResponding = UOS_FALSE;
        return UOS_TRUE;
    }

    bool_t send_ping() override
    {
        uint8_t ping_data[8] = {0xAA, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55};
        mIsResponding = UOS_FALSE;
        return mBus->send(0x100, ping_data, 8);
    }

    bool_t is_responding() const override
    {
        return mIsResponding;
    }

private:
    void on_can_message(uint16_t can_id, uint8_t *data, int32_t len)
    {
        if (can_id == 0x101 && len >= 2 && data[0] == 0xBB)
        {
            mIsResponding = UOS_TRUE;
        }
    }

    SimulatedCanBus *mBus;
    bool_t mIsResponding;
};

class LEDDisplayDevice : public IConnectableDevice
{
public:
    explicit LEDDisplayDevice(SimulatedCanBus *bus)
        : mBus(bus), mIsResponding(UOS_FALSE) {}

    string get_device_name() const override
    {
        return "led_display";
    }

    bool_t initialize() override
    {
        if (mBus == nullptr)
        {
            return UOS_FALSE;
        }

        mBus->register_handler(get_device_name(),
            [this](uint16_t can_id, uint8_t *data, int32_t len) {
                this->on_can_message(can_id, data, len);
            });
        return UOS_TRUE;
    }

    bool_t cleanup() override
    {
        if (mBus != nullptr)
        {
            mBus->unregister_handler(get_device_name());
        }
        mIsResponding = UOS_FALSE;
        return UOS_TRUE;
    }

    bool_t send_ping() override
    {
        uint8_t ping_data[8] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        mIsResponding = UOS_FALSE;
        return mBus->send(0x200, ping_data, 8);
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

    SimulatedCanBus *mBus;
    bool_t mIsResponding;
};

class AVASDevice : public IConnectableDevice
{
public:
    explicit AVASDevice(SimulatedCanBus *bus)
        : mBus(bus), mIsResponding(UOS_FALSE) {}

    string get_device_name() const override
    {
        return "avas";
    }

    bool_t initialize() override
    {
        if (mBus == nullptr)
        {
            return UOS_FALSE;
        }

        mBus->register_handler(get_device_name(),
            [this](uint16_t can_id, uint8_t *data, int32_t len) {
                this->on_can_message(can_id, data, len);
            });
        return UOS_TRUE;
    }

    bool_t cleanup() override
    {
        if (mBus != nullptr)
        {
            mBus->unregister_handler(get_device_name());
        }
        mIsResponding = UOS_FALSE;
        return UOS_TRUE;
    }

    bool_t send_ping() override
    {
        uint8_t ping_data[8] = {0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        mIsResponding = UOS_FALSE;
        return mBus->send(0x300, ping_data, 8);
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

    SimulatedCanBus *mBus;
    bool_t mIsResponding;
};

static void on_device_status_changed(const string &device_name,
                                     device_connection_status_t old_status,
                                     device_connection_status_t new_status)
{
    const char *status_str[] = {"UNKNOWN", "CHECKING", "ONLINE", "OFFLINE", "TIMEOUT"};
    printf("[Status] %s: %s -> %s\n",
        device_name.c_str(),
        status_str[old_status],
        status_str[new_status]);
}

int main()
{
    SimulatedCanBus bus;
    DeviceConnectionChecker checker;
    checker.set_status_callback(on_device_status_changed);

    shared_ptr<IConnectableDevice> autohook = make_shared<AutohookDevice>(&bus);
    shared_ptr<IConnectableDevice> led = make_shared<LEDDisplayDevice>(&bus);
    shared_ptr<IConnectableDevice> avas = make_shared<AVASDevice>(&bus);

    checker.register_device(autohook);
    checker.register_device(led);
    checker.register_device(avas);

    checker.start_check_all();

    for (int loop = 0; loop < 50; ++loop)
    {
        if (loop == 5)
        {
            uint8_t pong_data[8] = {0xBB, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            bus.simulate_receive(0x101, pong_data, 8);
        }

        if (loop == 10)
        {
            uint8_t pong_data[8] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            bus.simulate_receive(0x201, pong_data, 8);
        }

        if (loop == 15)
        {
            uint8_t pong_data[8] = {0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            bus.simulate_receive(0x301, pong_data, 8);
        }

        checker.update();
        usleep(100000);
    }

    return 0;
}
