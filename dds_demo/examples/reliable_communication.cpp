// Reliable communication example
// Demonstrates configuring QoS for reliable message delivery

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
        // Create custom configuration for reliable communication
        DDSConfig config;
        config.domain_id = 0;
        config.participant_name = "ReliableParticipant";
        config.reliability = ReliabilityKind::RELIABLE;
        config.durability = DurabilityKind::TRANSIENT_LOCAL;
        config.history_depth = 100;

        // Initialize with custom config
        DDSManager manager;
        if (!manager.initialize(config))
        {
            std::cerr << "Failed to initialize DDSManager" << std::endl;
            return 1;
        }

        std::cout << "DDSManager initialized with RELIABLE QoS" << std::endl;

        int received_count = 0;

        // Create subscriber
        auto subscriber = manager.createSubscriber<CommonMessage>("ReliableTopic",
            [&received_count](const CommonMessage& msg)
            {
                received_count++;
                std::cout << "Received [" << received_count << "]: "
                          << msg.content() << std::endl;
            });

        // Create publisher
        auto publisher = manager.createPublisher<CommonMessage>("ReliableTopic");

        std::cout << "Publisher and subscriber created with reliable QoS" << std::endl;

        // Wait for discovery
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Publish messages rapidly
        const int total_messages = 50;
        std::cout << "Publishing " << total_messages << " messages rapidly..." << std::endl;

        for (int i = 0; i < total_messages; ++i)
        {
            CommonMessage msg;
            msg.message_id("RELIABLE_" + std::to_string(i));
            msg.content("Reliable message #" + std::to_string(i));
            msg.timestamp(std::chrono::system_clock::now().time_since_epoch().count());
            msg.sequence_number(i);

            if (publisher->publish(msg))
            {
                if (i % 10 == 0)
                {
                    std::cout << "Published " << i << " messages" << std::endl;
                }
            }

            // Minimal delay
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::cout << "All messages published. Waiting for reception..." << std::endl;

        // Wait for all messages to be received
        std::this_thread::sleep_for(std::chrono::seconds(5));

        std::cout << "Sent: " << total_messages << " messages" << std::endl;
        std::cout << "Received: " << received_count << " messages" << std::endl;

        if (received_count == total_messages)
        {
            std::cout << "SUCCESS: All messages received reliably!" << std::endl;
        }
        else
        {
            std::cout << "WARNING: Some messages may have been lost" << std::endl;
        }
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
