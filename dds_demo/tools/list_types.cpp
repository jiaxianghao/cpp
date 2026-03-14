// List all available DDS message types
// Helps developers see what types are registered

#include <iostream>
#include <vector>
#include <string>

// Include all generated IDL types
#include "CommonMessage.hpp"
#include "SensorData.hpp"
#include "CommandMessage.hpp"

struct TypeInfo
{
    std::string name;
    std::string description;
    std::string idl_file;
};

int main()
{
    std::cout << "==========================================" << std::endl;
    std::cout << "Available DDS Message Types" << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << std::endl;

    // List of available types
    std::vector<TypeInfo> types =
    {
        {
            "CommonMessage",
            "General purpose message with id, content, timestamp, and sequence number",
            "idl/CommonMessage.idl"
        },
        {
            "SensorData",
            "Sensor data message with value, unit, and validation flag",
            "idl/SensorData.idl"
        },
        {
            "CommandMessage",
            "Command message with enum type, target, and parameters",
            "idl/CommandMessage.idl"
        }
    };

    // Display types
    int index = 1;
    for (const auto& type : types)
    {
        std::cout << index << ". " << type.name << std::endl;
        std::cout << "   Description: " << type.description << std::endl;
        std::cout << "   IDL File:    " << type.idl_file << std::endl;
        std::cout << std::endl;
        index++;
    }

    std::cout << "Total types available: " << types.size() << std::endl;
    std::cout << std::endl;

    std::cout << "Usage Examples:" << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << std::endl;

    std::cout << "// Create publisher" << std::endl;
    std::cout << "auto pub = manager.createPublisher<CommonMessage>(\"TopicName\");" << std::endl;
    std::cout << std::endl;

    std::cout << "// Create subscriber" << std::endl;
    std::cout << "auto sub = manager.createSubscriber<SensorData>(\"TopicName\"," << std::endl;
    std::cout << "    [](const SensorData& msg) {" << std::endl;
    std::cout << "        // Handle message" << std::endl;
    std::cout << "    });" << std::endl;
    std::cout << std::endl;

    std::cout << "To add a new type:" << std::endl;
    std::cout << "  1. Create IDL file in idl/ directory" << std::endl;
    std::cout << "  2. Run: cd build && cmake .. && make" << std::endl;
    std::cout << "  3. Use immediately with templates!" << std::endl;
    std::cout << std::endl;

    std::cout << "For help creating IDL files:" << std::endl;
    std::cout << "  python tools/create_idl.py --interactive" << std::endl;
    std::cout << std::endl;

    return 0;
}
