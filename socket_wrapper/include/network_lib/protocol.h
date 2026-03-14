#ifndef NETWORK_LIB_PROTOCOL_H
#define NETWORK_LIB_PROTOCOL_H

#include <vector>
#include <cstdint>
#include <cstddef>

namespace network_lib
{

// Base class for protocol abstraction
// Provides extension point for different protocol implementations
class Protocol
{
public:
    virtual ~Protocol() = default;

    // Encode data according to protocol
    // Returns encoded byte vector
    virtual std::vector<uint8_t> encode(const void* data, size_t len) = 0;

    // Decode data according to protocol
    // Returns decoded byte vector
    virtual std::vector<uint8_t> decode(const void* data, size_t len) = 0;
};

} // namespace network_lib

#endif // NETWORK_LIB_PROTOCOL_H

