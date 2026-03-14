/*
 * Tutorial: Adding a New Message Type
 * ====================================
 *
 * This example demonstrates the complete workflow for adding a new message type
 * to the DDS Wrapper framework. Follow these steps:
 *
 * STEP 1: Define your IDL file
 * -----------------------------
 * Create a new file: idl/RobotStatus.idl
 *
 * struct RobotStatus
 * {
 *     string robot_id;
 *     double position_x;
 *     double position_y;
 *     long battery_level;
 *     long timestamp;
 * };
 *
 * STEP 2: Rebuild the project
 * ----------------------------
 * cd build
 * cmake ..
 * make
 *
 * CMake will automatically detect the new IDL file and generate C++ code.
 *
 * STEP 3: Use in your code
 * ------------------------
 * Just include the generated header and use it with templates!
 */

#include "dds_wrapper/DDSManager.h"
#include "IDLTypes.h"  // Register all IDL types
#include <iostream>
#include <thread>
#include <chrono>

using namespace dds_wrapper;

// Example of how you would use a new custom type
void demonstrateCustomType()
{
    std::cout << "=== Custom Type Usage Tutorial ===" << std::endl;
    std::cout << "\nStep 1: Initialize DDSManager" << std::endl;

    DDSManager manager;
    if (!manager.initialize())
    {
        std::cerr << "Failed to initialize" << std::endl;
        return;
    }

    std::cout << "  ✓ DDSManager initialized" << std::endl;

    std::cout << "\nStep 2: Create Publisher for your custom type" << std::endl;
    std::cout << "  Code: auto pub = manager.createPublisher<YourType>(\"TopicName\");" << std::endl;

    // Using CommandMessage as example (replace with your type)
    auto pub = manager.createPublisher<CommandMessage>("CustomTopic");
    std::cout << "  ✓ Publisher created" << std::endl;

    std::cout << "\nStep 3: Create Subscriber with callback" << std::endl;
    std::cout << "  Code: auto sub = manager.createSubscriber<YourType>(\"TopicName\", callback);" << std::endl;

    auto sub = manager.createSubscriber<CommandMessage>("CustomTopic",
        [](const CommandMessage& msg)
        {
            std::cout << "  📨 Received custom message:" << std::endl;
            std::cout << "     Command ID: " << msg.command_id() << std::endl;
            std::cout << "     Target: " << msg.target_id() << std::endl;
            std::cout << "     Priority: " << msg.priority() << std::endl;
        });

    std::cout << "  ✓ Subscriber created with callback" << std::endl;

    std::cout << "\nStep 4: Publish messages" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (int i = 0; i < 3; ++i)
    {
        CommandMessage msg;
        msg.command_id("CMD_" + std::to_string(i));
        msg.command_type(CommandType::START);  // Using enum value
        msg.target_id("DEVICE_001");
        msg.parameters("{\"speed\": 100}");
        msg.timestamp(std::chrono::system_clock::now().time_since_epoch().count());
        msg.priority(i);

        if (pub->publish(msg))
        {
            std::cout << "  ✓ Published message " << i << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "\n=== Key Points ===" << std::endl;
    std::cout << "1. Define IDL → CMake auto-generates C++ code" << std::endl;
    std::cout << "2. No modification to wrapper code needed!" << std::endl;
    std::cout << "3. Templates handle any type automatically" << std::endl;
    std::cout << "4. Type-safe at compile time" << std::endl;
    std::cout << "5. Add 100 types? Still just 3 steps each!" << std::endl;
}

// Example showing multiple custom types together
void demonstrateMultipleCustomTypes()
{
    std::cout << "\n=== Using Multiple Custom Types ===" << std::endl;

    DDSManager manager;
    manager.initialize();

    // You can create publishers/subscribers for different types
    // all in the same application!

    auto commandPub = manager.createPublisher<CommandMessage>("Commands");
    auto sensorSub = manager.createSubscriber<SensorData>("Sensors",
        [](const SensorData& msg)
        {
            std::cout << "Sensor update: " << msg.value() << std::endl;
        });

    std::cout << "✓ Multiple types work seamlessly together" << std::endl;
}

int main()
{
    try
    {
        std::cout << "╔════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║  FastDDS Wrapper: Adding New Message Types        ║" << std::endl;
        std::cout << "║  Interactive Tutorial                             ║" << std::endl;
        std::cout << "╚════════════════════════════════════════════════════╝" << std::endl;

        demonstrateCustomType();
        demonstrateMultipleCustomTypes();

        std::cout << "\n✅ Tutorial completed!" << std::endl;
        std::cout << "\nNext steps:" << std::endl;
        std::cout << "  1. Try creating your own IDL file" << std::endl;
        std::cout << "  2. Run tools/create_idl.py for templates" << std::endl;
        std::cout << "  3. Check docs/ADD_NEW_TYPE.md for details" << std::endl;
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
