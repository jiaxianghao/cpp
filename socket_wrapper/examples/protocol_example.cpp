#include "network_lib/protocol.h"
#include "simple_protocol.h"
#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>

// Example of a custom protocol implementation
// This protocol adds a header with length information
class LengthHeaderProtocol : public network_lib::Protocol
{
public:
    std::vector<uint8_t> encode(const void* data, size_t len) override
    {
        std::vector<uint8_t> result;
        // Add 4-byte length header (big-endian)
        result.push_back((len >> 24) & 0xFF);
        result.push_back((len >> 16) & 0xFF);
        result.push_back((len >> 8) & 0xFF);
        result.push_back(len & 0xFF);
        // Add data
        result.insert(result.end(),
                      static_cast<const uint8_t*>(data),
                      static_cast<const uint8_t*>(data) + len);
        return result;
    }

    std::vector<uint8_t> decode(const void* data, size_t len) override
    {
        if (len < 4)
        {
            return std::vector<uint8_t>();
        }

        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        // Extract length from header
        size_t dataLen = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];

        if (dataLen + 4 > len)
        {
            return std::vector<uint8_t>();
        }

        // Return data without header
        std::vector<uint8_t> result(dataLen);
        std::memcpy(result.data(), bytes + 4, dataLen);
        return result;
    }
};

int main()
{
    std::cout << "Protocol Extension Example" << std::endl;
    std::cout << "==========================" << std::endl;

    // Example 1: SimpleProtocol (pass-through)
    {
        std::cout << "\n1. SimpleProtocol (pass-through):" << std::endl;
        SimpleProtocol protocol;
        std::string testData = "Hello, World!";
        auto encoded = protocol.encode(testData.c_str(), testData.length());
        auto decoded = protocol.decode(encoded.data(), encoded.size());
        std::string result(reinterpret_cast<const char*>(decoded.data()), decoded.size());
        std::cout << "   Original: " << testData << std::endl;
        std::cout << "   Decoded:  " << result << std::endl;
    }

    // Example 2: LengthHeaderProtocol (with header)
    {
        std::cout << "\n2. LengthHeaderProtocol (with length header):" << std::endl;
        LengthHeaderProtocol protocol;
        std::string testData = "Test Message";
        auto encoded = protocol.encode(testData.c_str(), testData.length());
        auto decoded = protocol.decode(encoded.data(), encoded.size());
        std::string result(reinterpret_cast<const char*>(decoded.data()), decoded.size());
        std::cout << "   Original: " << testData << std::endl;
        std::cout << "   Encoded size: " << encoded.size() << " bytes" << std::endl;
        std::cout << "   Decoded:  " << result << std::endl;
    }

    std::cout << "\nThis demonstrates how to extend the Protocol interface" << std::endl;
    std::cout << "to implement custom encoding/decoding schemes." << std::endl;

    return 0;
}
