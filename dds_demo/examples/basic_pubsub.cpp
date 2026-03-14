// Basic publisher-subscriber example
// Demonstrates simple message publishing and receiving

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
        if (!manager.initialize("../../config/default_config.json"))
        {
            std::cerr << "Failed to initialize DDSManager" << std::endl;
            return 1;
        }

        std::cout << "DDSManager initialized successfully" << std::endl;

        // Create Subscriber with callback
        auto subscriber = manager.createSubscriber<CommonMessage>("BasicTopic",
            [](const CommonMessage& msg)
            {
                std::cout << "Received message:" << std::endl;
                std::cout << "  ID: " << msg.message_id() << std::endl;
                std::cout << "  Content: " << msg.content() << std::endl;
                std::cout << "  Timestamp: " << msg.timestamp() << std::endl;
                std::cout << "  Sequence: " << msg.sequence_number() << std::endl;
            });

        std::cout << "Subscriber created" << std::endl;

        // Create Publisher
        auto publisher = manager.createPublisher<CommonMessage>("BasicTopic");
        std::cout << "Publisher created" << std::endl;

        // Give some time for discovery
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Publish messages
        for (int i = 0; i < 10; ++i)
        {
            CommonMessage msg;
            msg.message_id("MSG_" + std::to_string(i));
            msg.content("Hello from FastDDS Wrapper!");
            msg.timestamp(std::chrono::system_clock::now().time_since_epoch().count());
            msg.sequence_number(i);

            if (publisher->publish(msg))
            {
                std::cout << "Published message " << i << std::endl;
            }
            else
            {
                std::cerr << "Failed to publish message " << i << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        // Wait for messages to be received
        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::cout << "Basic pub-sub example completed" << std::endl;
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
