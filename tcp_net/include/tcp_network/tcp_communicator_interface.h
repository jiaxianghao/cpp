#pragma once

#include <vector>
#include <functional>
#include <cstdint>

namespace tcp_network {

// Connection result enumeration for better error handling
enum class ConnectionResult {
    SUCCESS = 0,
    INVALID_ADDRESS,
    CONNECTION_REFUSED, 
    TIMEOUT,
    NETWORK_ERROR,
    SOCKET_ERROR,
    ALREADY_CONNECTED
};

// Enhanced base interface for TCP communication
class ITcpCommunicator {
public:
    using ResponseCallback = std::function<void(const std::vector<uint8_t>&)>;
    using ErrorCallback = std::function<void(ConnectionResult)>;
    
    virtual ~ITcpCommunicator() = default;
    
    // Core interface methods
    virtual bool connect_to_server(const std::string& ip, int32_t port) = 0;
    virtual void disconnect_from_server() = 0;
    virtual bool is_connected() const = 0;
    virtual bool send_data(const std::vector<uint8_t>& data) = 0;
    virtual void register_global_callback(ResponseCallback cb) = 0;
    virtual void update() = 0;
    
    // Enhanced interface methods
    virtual ConnectionResult get_last_connection_result() const = 0;
    virtual bool is_connection_healthy() const = 0;
};

} // namespace tcp_network