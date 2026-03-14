#include "simple_protocol.h"
#include <cstring>

std::vector<uint8_t> SimpleProtocol::encode(const void* data, size_t len)
{
    std::vector<uint8_t> result(len);
    std::memcpy(result.data(), data, len);
    return result;
}

std::vector<uint8_t> SimpleProtocol::decode(const void* data, size_t len)
{
    std::vector<uint8_t> result(len);
    std::memcpy(result.data(), data, len);
    return result;
}

