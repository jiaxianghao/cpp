// Multiple topics example
// Demonstrates using multiple publishers and subscribers simultaneously

#include "dds_wrapper/DDSManager.h"
#include "IDLTypes.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace dds_wrapper;

int main()
{
    try
    {
        // Initialize DDS Manager
        DDSManager manager;
        if (!manager.initialize())
        {
            std::cerr << "Failed to initialize DDSManager" << std::endl;
            return 1;
        }

        std::cout << "DDSManager initialized" << std::endl;

        // Create multiple subscribers
        auto commonSub = manager.createSubscriber<CommonMessage>("CommonTopic",
            [](const CommonMessage& msg)
            {
                std::cout << "[CommonTopic] Received: " << msg.content() << std::endl;
            });

        auto sensorSub = manager.createSubscriber<SensorData>("SensorTopic",
            [](const SensorData& msg)
            {
                std::cout << "[SensorTopic] Sensor: " << msg.sensor_id()
                          << ", Value: " << msg.value()
                          << " " << msg.unit() << std::endl;
            });

        // Create multiple publishers
        auto commonPub = manager.createPublisher<CommonMessage>("CommonTopic");
        auto sensorPub = manager.createPublisher<SensorData>("SensorTopic");

        std::cout << "Publishers and subscribers created" << std::endl;

        // Wait for discovery
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Publish to multiple topics
        for (int i = 0; i < 5; ++i)
        {
            // Publish common message
            CommonMessage commonMsg;
            commonMsg.message_id("COMMON_" + std::to_string(i));
            commonMsg.content("Multi-topic message " + std::to_string(i));
            commonMsg.timestamp(std::chrono::system_clock::now().time_since_epoch().count());
            commonMsg.sequence_number(i);
            commonPub->publish(commonMsg);

            // Publish sensor data
            SensorData sensorMsg;
            sensorMsg.sensor_id("SENSOR_001");
            sensorMsg.sensor_type("Temperature");
            sensorMsg.value(20.0 + i * 0.5);
            sensorMsg.unit("Celsius");
            sensorMsg.timestamp(std::chrono::system_clock::now().time_since_epoch().count());
            sensorMsg.is_valid(true);
            sensorPub->publish(sensorMsg);

            std::cout << "Published message set " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Wait for messages to be received
        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::cout << "Multiple topics example completed" << std::endl;
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
