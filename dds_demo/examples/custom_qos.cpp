// Custom QoS example
// Demonstrates different QoS configurations

#include "dds_wrapper/DDSManager.h"
#include "IDLTypes.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace dds_wrapper;

void testBestEffort()
{
    std::cout << "\n=== Testing BEST_EFFORT QoS ===" << std::endl;

    DDSConfig config;
    config.reliability = ReliabilityKind::BEST_EFFORT;
    config.durability = DurabilityKind::VOLATILE;
    config.history_depth = 1;

    DDSManager manager;
    manager.initialize(config);

    auto pub = manager.createPublisher<SensorData>("BestEffortTopic");
    auto sub = manager.createSubscriber<SensorData>("BestEffortTopic",
        [](const SensorData& msg)
        {
            std::cout << "  [BEST_EFFORT] Sensor " << msg.sensor_id()
                      << ": " << msg.value() << std::endl;
        });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (int i = 0; i < 5; ++i)
    {
        SensorData msg;
        msg.sensor_id("TEMP_001");
        msg.sensor_type("Temperature");
        msg.value(20.0 + i);
        msg.unit("C");
        msg.timestamp(std::chrono::system_clock::now().time_since_epoch().count());
        msg.is_valid(true);
        pub->publish(msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void testReliable()
{
    std::cout << "\n=== Testing RELIABLE QoS ===" << std::endl;

    DDSConfig config;
    config.reliability = ReliabilityKind::RELIABLE;
    config.durability = DurabilityKind::TRANSIENT_LOCAL;
    config.history_depth = 10;

    DDSManager manager;
    manager.initialize(config);

    auto pub = manager.createPublisher<SensorData>("ReliableTopic");
    auto sub = manager.createSubscriber<SensorData>("ReliableTopic",
        [](const SensorData& msg)
        {
            std::cout << "  [RELIABLE] Sensor " << msg.sensor_id()
                      << ": " << msg.value() << std::endl;
        });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (int i = 0; i < 5; ++i)
    {
        SensorData msg;
        msg.sensor_id("PRESS_001");
        msg.sensor_type("Pressure");
        msg.value(100.0 + i * 5);
        msg.unit("kPa");
        msg.timestamp(std::chrono::system_clock::now().time_since_epoch().count());
        msg.is_valid(true);
        pub->publish(msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void testLateJoiner()
{
    std::cout << "\n=== Testing Late Joiner (TRANSIENT_LOCAL) ===" << std::endl;

    DDSConfig config;
    config.reliability = ReliabilityKind::RELIABLE;
    config.durability = DurabilityKind::TRANSIENT_LOCAL;
    config.history_depth = 10;

    DDSManager manager;
    manager.initialize(config);

    auto pub = manager.createPublisher<SensorData>("LateJoinerTopic");

    // Publish messages BEFORE subscriber joins
    std::cout << "Publishing messages before subscriber exists..." << std::endl;
    for (int i = 0; i < 3; ++i)
    {
        SensorData msg;
        msg.sensor_id("HUMID_001");
        msg.sensor_type("Humidity");
        msg.value(50.0 + i * 5);
        msg.unit("%");
        msg.timestamp(std::chrono::system_clock::now().time_since_epoch().count());
        msg.is_valid(true);
        pub->publish(msg);
        std::cout << "  Published message " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Now create subscriber - it should receive historical messages
    std::cout << "Creating late-joining subscriber..." << std::endl;
    int count = 0;
    auto sub = manager.createSubscriber<SensorData>("LateJoinerTopic",
        [&count](const SensorData& msg)
        {
            count++;
            std::cout << "  [LATE_JOINER] Received historical message " << count
                      << ": " << msg.value() << msg.unit() << std::endl;
        });

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Late joiner received " << count << " historical messages" << std::endl;
}

int main()
{
    try
    {
        std::cout << "Custom QoS Examples" << std::endl;
        std::cout << "===================" << std::endl;

        testBestEffort();
        testReliable();
        testLateJoiner();

        std::cout << "\nAll QoS examples completed successfully" << std::endl;
    }
    catch (const DDSException& e)
    {
        std::cerr << "DDS Exception: " << e.getMessage() << std::endl;
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
