#ifndef SIMPLE_PROTOCOL_H
#define SIMPLE_PROTOCOL_H

#include "network_lib/protocol.h"
#include <vector>
#include <cstdint>
#include <cstddef>

// Simple protocol implementation that passes data through without modification
// This is an example of how to extend the Protocol interface
class SimpleProtocol : public network_lib::Protocol
{
public:
    std::vector<uint8_t> encode(const void* data, size_t len) override;
    std::vector<uint8_t> decode(const void* data, size_t len) override;
};

#endif // SIMPLE_PROTOCOL_H

