#pragma once

#include "tcp_communicator_interface.h"
#include "heartbeat_strategy.h"
#include "message_boundary_detector.h"
#include <chrono>
#include <atomic>

namespace tcp_network {

// Connection configuration structure
struct ConnectionConfig {
    uint32_t connect_timeout_ms = 5000;   // Connection timeout
    uint32_t read_timeout_ms = 3000;      // Read/write timeout  
    uint32_t retry_count = 3;             // Maximum retry attempts
    uint32_t retry_interval_ms = 1000;    // Interval between retries
    bool enable_keepalive = true;         // Enable TCP keepalive
    uint32_t keepalive_idle = 60;         // Keepalive idle time
    uint32_t keepalive_interval = 10;     // Keepalive probe interval
    uint32_t keepalive_count = 3;         // Max keepalive probes
    
    // Message boundary detection configuration
    MessageBoundaryConfig message_boundary = MessageBoundaryConfig::newline_delimited();
    
    // Heartbeat configuration
    std::unique_ptr<IHeartbeatStrategy> heartbeat_strategy;
    uint32_t heartbeat_interval_ms = 30000;      // Heartbeat interval in milliseconds
    uint32_t heartbeat_timeout_ms = 5000;        // Heartbeat response timeout
    
    // Default constructor
    ConnectionConfig() = default;
    
    // Move constructor and assignment
    ConnectionConfig(ConnectionConfig&& other) noexcept = default;
    ConnectionConfig& operator=(ConnectionConfig&& other) noexcept = default;
    
    // Copy constructor and assignment
    ConnectionConfig(const ConnectionConfig& other);
    ConnectionConfig& operator=(const ConnectionConfig& other);
};

} // namespace tcp_network