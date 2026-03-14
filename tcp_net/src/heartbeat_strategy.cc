#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <tcp_network/heartbeat_strategy.h>

using namespace tcp_network;

// Factory function implementation
std::unique_ptr<IHeartbeatStrategy> tcp_network::create_heartbeat_strategy(HeartbeatType type) {
    switch (type) {
        case HeartbeatType::SOCKET_STATUS:
            return std::make_unique<SocketStatusStrategy>();
        case HeartbeatType::EMPTY_PACKET:
            return std::make_unique<EmptyPacketStrategy>();
        case HeartbeatType::DISABLED:
            return std::make_unique<DisabledStrategy>();
        default:
            return std::make_unique<DisabledStrategy>();  // Default to disabled
    }
}

// SocketStatusStrategy implementation
bool SocketStatusStrategy::execute_heartbeat(int socket_fd) {
    if (socket_fd < 0) {
        return false;
    }
    
    // Use poll() to check if socket is writable without blocking
    struct pollfd pfd;
    pfd.fd = socket_fd;
    pfd.events = POLLOUT;
    pfd.revents = 0;
    
    // Very short timeout for status check (10ms)
    int result = poll(&pfd, 1, 10);
    
    if (result < 0) {
        std::cout << "[WARN] Socket status check failed: " << strerror(errno) << "\n";
        return false;
    }
    
    if (result == 0) {
        // Timeout - socket not immediately writable, but still connected
        return true;
    }
    
    if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
        std::cout << "[WARN] Socket error detected in status check\n";
        return false;
    }
    
    // Socket is writable and healthy
    return true;
}

std::string SocketStatusStrategy::get_strategy_name() const {
    return "SOCKET_STATUS";
}

std::unique_ptr<IHeartbeatStrategy> SocketStatusStrategy::clone() const {
    return std::make_unique<SocketStatusStrategy>();
}

bool SocketStatusStrategy::requires_network_activity() const {
    return false;
}

// EmptyPacketStrategy implementation
bool EmptyPacketStrategy::execute_heartbeat(int socket_fd) {
    if (socket_fd < 0) {
        return false;
    }
    
    // Send a 0-byte packet to test connectivity
    ssize_t sent = send(socket_fd, nullptr, 0, MSG_NOSIGNAL | MSG_DONTWAIT);
    
    if (sent < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Socket busy but still connected
            return true;
        }
        
        std::cout << "[WARN] Empty packet heartbeat failed: " << strerror(errno) << "\n";
        return false;
    }
    
    return true;
}

std::string EmptyPacketStrategy::get_strategy_name() const {
    return "EMPTY_PACKET";
}

std::unique_ptr<IHeartbeatStrategy> EmptyPacketStrategy::clone() const {
    return std::make_unique<EmptyPacketStrategy>();
}

bool EmptyPacketStrategy::requires_network_activity() const {
    return true;
}

// DisabledStrategy implementation
bool DisabledStrategy::execute_heartbeat(int socket_fd) {
    (void)socket_fd; // Suppress unused parameter warning
    // Heartbeat is disabled, always return true
    return true;
}

std::string DisabledStrategy::get_strategy_name() const {
    return "DISABLED";
}

std::unique_ptr<IHeartbeatStrategy> DisabledStrategy::clone() const {
    return std::make_unique<DisabledStrategy>();
}

bool DisabledStrategy::requires_network_activity() const {
    return false;
}