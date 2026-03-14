#pragma once

#include <memory>
#include <vector>
#include <string>
#include <cstdint>

namespace tcp_network {

// Forward declaration
class IHeartbeatStrategy;

// Heartbeat strategy types
enum class HeartbeatType {
    SOCKET_STATUS,
    EMPTY_PACKET,
    DISABLED
};

// Factory function for creating heartbeat strategies
std::unique_ptr<IHeartbeatStrategy> create_heartbeat_strategy(HeartbeatType type);

// Abstract base class for heartbeat strategies
class IHeartbeatStrategy {
public:
    virtual ~IHeartbeatStrategy() = default;
    
    // Execute heartbeat check and return success status
    virtual bool execute_heartbeat(int socket_fd) = 0;
    
    // Get strategy description for debugging/logging
    virtual std::string get_strategy_name() const = 0;
    
    // Clone method for copying strategy instances
    virtual std::unique_ptr<IHeartbeatStrategy> clone() const = 0;
    
    // Check if this strategy requires network activity
    virtual bool requires_network_activity() const = 0;
};

// Socket status check heartbeat strategy
class SocketStatusStrategy : public IHeartbeatStrategy {
public:
    bool execute_heartbeat(int socket_fd) override;
    std::string get_strategy_name() const override;
    std::unique_ptr<IHeartbeatStrategy> clone() const override;
    bool requires_network_activity() const override;
};

// Empty packet heartbeat strategy
class EmptyPacketStrategy : public IHeartbeatStrategy {
public:
    bool execute_heartbeat(int socket_fd) override;
    std::string get_strategy_name() const override;
    std::unique_ptr<IHeartbeatStrategy> clone() const override;
    bool requires_network_activity() const override;
};

// Disabled heartbeat strategy
class DisabledStrategy : public IHeartbeatStrategy {
public:
    bool execute_heartbeat(int socket_fd) override;
    std::string get_strategy_name() const override;
    std::unique_ptr<IHeartbeatStrategy> clone() const override;
    bool requires_network_activity() const override;
};

} // namespace tcp_network