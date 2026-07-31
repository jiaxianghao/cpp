/*
 * Copyright 2016-2025. UISEE TECHNOLOGIES (BEIJING) LTD. All rights reserved.
 * See LICENSE AGREEMENT file in the project root for full license information.
 */

#include <gtest/gtest.h>
#include "../include/device_connection_checker.h"
#include <vector>
#include <sstream>

// Mock logger for testing
class MockLogger : public ILogger
{
public:
    void log_error(const std::string& tag, const std::string& message) override {
        errors.push_back("[" + tag + "] " + message);
    }

    void log_warning(const std::string& tag, const std::string& message) override {
        warnings.push_back("[" + tag + "] " + message);
    }

    void log_info(const std::string& tag, const std::string& message) override {
        infos.push_back("[" + tag + "] " + message);
    }

    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    std::vector<std::string> infos;
};

// Mock time provider for testing
class MockTimeProvider : public ITimeProvider
{
public:
    MockTimeProvider() : current_time(0.0) {}

    float64_t get_current_time() const override {
        return current_time;
    }

    void advance_time(float64_t delta) {
        current_time += delta;
    }

    void set_time(float64_t time) {
        current_time = time;
    }

private:
    float64_t current_time;
};



// Test device implementing IConnectableDevice interface
class TestConnectableDevice : public IConnectableDevice
{
public:
    explicit TestConnectableDevice(const std::string &name)
        : device_name(name),
          mIsResponding(UOS_FALSE),
          send_ping_called(false),
          status_callback_called(false),
          last_old_status(DEVICE_STATUS_UNKNOWN),
          last_new_status(DEVICE_STATUS_UNKNOWN) {}

    std::string get_device_name() const override {
        return device_name;
    }

    bool_t initialize() override {
        mIsResponding = UOS_FALSE;
        return UOS_TRUE;
    }

    bool_t cleanup() override {
        mIsResponding = UOS_FALSE;
        return UOS_TRUE;
    }

    bool_t send_ping() override {
        send_ping_called = true;
        mIsResponding = UOS_FALSE;
        return UOS_TRUE;
    }

    bool_t is_responding() const override {
        return mIsResponding;
    }

    void simulate_pong(uint8_t pattern) {
        if (pattern == 0xBB) {
            mIsResponding = UOS_TRUE;
        }
    }

    void on_status_changed(device_connection_status_t old_status,
                           device_connection_status_t new_status) override {
        status_callback_called = true;
        last_old_status = old_status;
        last_new_status = new_status;
    }

    std::string device_name;
    bool_t mIsResponding;
    bool send_ping_called;
    bool status_callback_called;
    device_connection_status_t last_old_status;
    device_connection_status_t last_new_status;
};

// Test fixture
class DeviceConnectionCheckerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        checker = new DeviceConnectionChecker();
    }

    void TearDown() override
    {
        delete checker;
    }

    DeviceConnectionChecker *checker;
};

// Test fixture with mocked dependencies
class DeviceConnectionCheckerMockTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mock_logger = std::make_shared<MockLogger>();
        mock_time_provider = std::make_shared<MockTimeProvider>();
        checker = new DeviceConnectionChecker(mock_logger, mock_time_provider);
    }

    void TearDown() override
    {
        delete checker;
    }

    DeviceConnectionChecker *checker;
    std::shared_ptr<MockLogger> mock_logger;
    std::shared_ptr<MockTimeProvider> mock_time_provider;
};

// Test: Register device
TEST_F(DeviceConnectionCheckerTest, RegisterDevice)
{
    auto device = std::make_shared<TestConnectableDevice>("test_device");
    EXPECT_EQ(UOS_TRUE, checker->register_device(device));
    EXPECT_EQ(DEVICE_STATUS_UNKNOWN, checker->get_device_status("test_device"));
}

// Test: Register device with null pointer
TEST_F(DeviceConnectionCheckerTest, RegisterDeviceNullPointer)
{
    EXPECT_EQ(UOS_FALSE, checker->register_device(nullptr));
}

// Test: Register duplicate device
TEST_F(DeviceConnectionCheckerTest, RegisterDuplicateDevice)
{
    auto device = std::make_shared<TestConnectableDevice>("test_device");
    EXPECT_EQ(UOS_TRUE, checker->register_device(device));
    EXPECT_EQ(UOS_FALSE, checker->register_device(device));
}

// Test: Start check sends ping
TEST_F(DeviceConnectionCheckerTest, StartCheckSendsPing)
{
    auto device = std::make_shared<TestConnectableDevice>("test_device");
    checker->register_device(device);

    EXPECT_EQ(UOS_TRUE, checker->start_check("test_device"));
    EXPECT_TRUE(device->send_ping_called);
    EXPECT_EQ(DEVICE_STATUS_CHECKING, checker->get_device_status("test_device"));
}

// Test: Start check non-existent device
TEST_F(DeviceConnectionCheckerTest, StartCheckNonExistentDevice)
{
    EXPECT_EQ(UOS_FALSE, checker->start_check("non_existent"));
}

// Test: Device reports pong and becomes online during update
TEST_F(DeviceConnectionCheckerTest, DeviceBecomesOnlineOnUpdate)
{
    auto device = std::make_shared<TestConnectableDevice>("test_device");
    checker->register_device(device);
    checker->start_check("test_device");

    device->simulate_pong(0xBB);
    checker->update();

    EXPECT_EQ(DEVICE_STATUS_ONLINE, checker->get_device_status("test_device"));
}

// Test: Device keeps checking when response pattern mismatches
TEST_F(DeviceConnectionCheckerTest, DeviceRemainsCheckingWhenNoResponse)
{
    auto device = std::make_shared<TestConnectableDevice>("test_device");
    checker->register_device(device);
    checker->start_check("test_device");

    device->simulate_pong(0xCC);
    checker->update();

    EXPECT_EQ(DEVICE_STATUS_CHECKING, checker->get_device_status("test_device"));
}



// Test: Status callback
TEST_F(DeviceConnectionCheckerTest, StatusCallback)
{
    auto device = std::make_shared<TestConnectableDevice>("test_device");
    checker->register_device(device);
    checker->start_check("test_device");

    device->simulate_pong(0xBB);
    checker->update();

    EXPECT_TRUE(device->send_ping_called);
    EXPECT_TRUE(device->status_callback_called);
    EXPECT_EQ(DEVICE_STATUS_CHECKING, device->last_old_status);
    EXPECT_EQ(DEVICE_STATUS_ONLINE, device->last_new_status);
}

// Test: Start check all devices
TEST_F(DeviceConnectionCheckerTest, StartCheckAll)
{
    auto device1 = std::make_shared<TestConnectableDevice>("device1");
    auto device2 = std::make_shared<TestConnectableDevice>("device2");

    checker->register_device(device1);
    checker->register_device(device2);

    EXPECT_EQ(UOS_TRUE, checker->start_check_all());
    EXPECT_EQ(DEVICE_STATUS_CHECKING, checker->get_device_status("device1"));
    EXPECT_EQ(DEVICE_STATUS_CHECKING, checker->get_device_status("device2"));
}

// Test: Get status of non-existent device
TEST_F(DeviceConnectionCheckerTest, GetStatusNonExistent)
{
    EXPECT_EQ(DEVICE_STATUS_UNKNOWN, checker->get_device_status("non_existent"));
}

// ============ Tests with Mocked Dependencies ============

// Test: Logger dependency injection
TEST_F(DeviceConnectionCheckerMockTest, LoggerDependencyInjection)
{
    auto device = std::make_shared<TestConnectableDevice>("test_device");
    checker->register_device(device);

    // Should have logged device registration
    EXPECT_FALSE(mock_logger->infos.empty());
    EXPECT_NE(mock_logger->infos[0].find("Registered device"), std::string::npos);
}

// Test: Time provider dependency injection
TEST_F(DeviceConnectionCheckerMockTest, TimeProviderDependencyInjection)
{
    auto device = std::make_shared<TestConnectableDevice>("test_device");
    checker->register_device(device);

    mock_time_provider->set_time(100.0);
    checker->start_check("test_device");

    // Simulate device response
    device->simulate_pong(0xBB);
    mock_time_provider->set_time(100.1); // 100ms later
    checker->update();

    EXPECT_EQ(DEVICE_STATUS_ONLINE, checker->get_device_status("test_device"));
}

// Test: Global status callback
TEST_F(DeviceConnectionCheckerMockTest, GlobalStatusCallback)
{
    bool callback_called = false;
    std::string callback_device_name;
    device_connection_status_t callback_old_status = DEVICE_STATUS_UNKNOWN;
    device_connection_status_t callback_new_status = DEVICE_STATUS_UNKNOWN;

    checker->set_status_callback([&](const std::string& name,
                                     device_connection_status_t old_status,
                                     device_connection_status_t new_status) {
        callback_called = true;
        callback_device_name = name;
        callback_old_status = old_status;
        callback_new_status = new_status;
    });

    auto device = std::make_shared<TestConnectableDevice>("test_device");
    checker->register_device(device);
    checker->start_check("test_device");

    device->simulate_pong(0xBB);
    checker->update();

    EXPECT_TRUE(callback_called);
    EXPECT_EQ("test_device", callback_device_name);
    EXPECT_EQ(DEVICE_STATUS_CHECKING, callback_old_status);
    EXPECT_EQ(DEVICE_STATUS_ONLINE, callback_new_status);
}

// Test: Timeout with mocked time
TEST_F(DeviceConnectionCheckerMockTest, TimeoutWithMockedTime)
{
    auto device = std::make_shared<TestConnectableDevice>("test_device");
    checker->register_device(device);

    mock_time_provider->set_time(0.0);
    checker->start_check("test_device");

    // Advance time beyond timeout (default 2.0s) and max retries (default 3)
    for (int i = 0; i < 3; i++) {
        mock_time_provider->advance_time(2.5); // Exceed timeout
        checker->update();
    }

    EXPECT_EQ(DEVICE_STATUS_OFFLINE, checker->get_device_status("test_device"));
}

// Test: Error logging for null device
TEST_F(DeviceConnectionCheckerMockTest, ErrorLoggingForNullDevice)
{
    checker->register_device(nullptr);

    EXPECT_FALSE(mock_logger->errors.empty());
    EXPECT_NE(mock_logger->errors[0].find("Device pointer is null"), std::string::npos);
}

// Test: Custom check configuration
TEST_F(DeviceConnectionCheckerMockTest, CustomCheckConfiguration)
{
    // Create custom configuration with shorter timeout
    CheckConfiguration custom_config(1.0, 1.5, 2);  // 1s timeout, 1.5s retry interval, 2 max retries

    auto device = std::make_shared<TestConnectableDevice>("test_device");
    checker->register_device(device, custom_config);

    // Verify configuration is logged
    EXPECT_FALSE(mock_logger->infos.empty());
    EXPECT_NE(mock_logger->infos[0].find("timeout=1s"), std::string::npos);
    EXPECT_NE(mock_logger->infos[0].find("retry=2"), std::string::npos);

    // Test timeout with custom configuration
    mock_time_provider->set_time(0.0);
    checker->start_check("test_device");

    // First timeout after 1s
    mock_time_provider->advance_time(1.5);
    checker->update();
    EXPECT_EQ(DEVICE_STATUS_CHECKING, checker->get_device_status("test_device"));

    // Second timeout after another 1.5s - should go offline (max 2 retries)
    mock_time_provider->advance_time(1.5);
    checker->update();
    EXPECT_EQ(DEVICE_STATUS_OFFLINE, checker->get_device_status("test_device"));
}

// Test: Different configurations for different devices
TEST_F(DeviceConnectionCheckerMockTest, DifferentConfigurationsPerDevice)
{
    CheckConfiguration fast_config(0.5, 1.0, 1);   // Fast device: 0.5s timeout, 1 retry
    CheckConfiguration slow_config(5.0, 10.0, 5);  // Slow device: 5s timeout, 5 retries

    auto fast_device = std::make_shared<TestConnectableDevice>("fast_device");
    auto slow_device = std::make_shared<TestConnectableDevice>("slow_device");

    checker->register_device(fast_device, fast_config);
    checker->register_device(slow_device, slow_config);

    // Verify both devices registered with different configs
    EXPECT_GE(mock_logger->infos.size(), 2);
    EXPECT_NE(mock_logger->infos[0].find("timeout=0.5s"), std::string::npos);
    EXPECT_NE(mock_logger->infos[1].find("timeout=5s"), std::string::npos);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

