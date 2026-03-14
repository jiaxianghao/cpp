/*
 * Copyright 2016-2024. UISEE TECHNOLOGIES (BEIJING) LTD. All rights reserved.
 * See LICENSE AGREEMENT file in the project root for full license information.
 */

#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <memory>
#include <algorithm>
#include <cstdint>
#include <iostream>

#include <tcp_network/tcp_communicator.h>

using namespace tcp_network;


TcpCommunicatorImpl::TcpCommunicatorImpl()
    : mSocketFd(-1)
    , mIsConnected(false)
    , mIsHealthy(false)
    , mLastConnectionResult(ConnectionResult::SUCCESS)
    , mServerPort(0)
    , mQueueSize(0)
    , mReceiveEnabled(false)
    , mHeartbeatEnabled(false)
    , mBytesSent(0)
    , mBytesReceived(0)
    , mConnectionErrors(0)
{
    mLastHeartbeat = std::chrono::steady_clock::now();
    mLastDataReceived = std::chrono::steady_clock::now();
}

TcpCommunicatorImpl::~TcpCommunicatorImpl() {
    disconnect_from_server();
}

bool TcpCommunicatorImpl::connect_to_server(const std::string& ip, int32_t port) {
    std::lock_guard<std::mutex> lock(mMutex);
    
    if (mIsConnected.load()) {
        std::cout << "[WARN] Already connected, disconnecting first\n";
        disconnect_from_server();
    }
    
    mServerIp = ip;
    mServerPort = port;
    
    // Validate IP address using inet_pton instead of deprecated inet_addr
    struct sockaddr_in addr;
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1) {
        std::cout << "[ERROR] Invalid IP address: " << ip << "\n";
        mLastConnectionResult = ConnectionResult::INVALID_ADDRESS;
        return false;
    }
    
    // Create socket with retry mechanism
    for (uint32_t attempt = 0; attempt < mConfig.retry_count; ++attempt) {
        mLastConnectionResult = create_socket();
        if (mLastConnectionResult != ConnectionResult::SUCCESS) {
            if (attempt < mConfig.retry_count - 1) {
                std::cout << "[WARN] Socket creation failed, retrying in " << mConfig.retry_interval_ms << "ms (attempt " << (attempt + 1) << "/" << mConfig.retry_count << ")\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(mConfig.retry_interval_ms));
            }
            continue;
        }
        
        // Configure socket
        if (!configure_socket()) {
            close_socket();
            mLastConnectionResult = ConnectionResult::SOCKET_ERROR;
            continue;
        }
        
        // Attempt connection
        mLastConnectionResult = perform_connect(ip, port);
        if (mLastConnectionResult == ConnectionResult::SUCCESS) {
            mIsConnected.store(true);
            mIsHealthy.store(true);
            mConnectionErrors.store(0);
            
            // Start background threads
            start_receive_thread();
            if(mConfig.heartbeat_strategy) {
                start_heartbeat_thread();
            }
            
            std::cout << "[INFO] Successfully connected to " << ip << ":" << port << "\n";
            return true;
        }
        
        close_socket();
        
        if (attempt < mConfig.retry_count - 1) {
            std::cout << "[WARN] Connection failed, retrying in " << mConfig.retry_interval_ms << "ms (attempt " << (attempt + 1) << "/" << mConfig.retry_count << ")\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(mConfig.retry_interval_ms));
        }
    }
    
    std::cout << "[ERROR] Failed to connect to " << ip << ":" << port << " after " << mConfig.retry_count << " attempts\n";
    return false;
}

void TcpCommunicatorImpl::disconnect_from_server() {
    std::lock_guard<std::mutex> lock(mMutex);
    
    if (!mIsConnected.load()) {
        return;
    }
    
    std::cout << "[INFO] Disconnecting from server\n";
    
    // Stop threads first
    stop_receive_thread();
    stop_heartbeat_thread();
    
    // Update connection state
    mIsConnected.store(false);
    mIsHealthy.store(false);
    
    // Close socket
    close_socket();
    
    // Clear data queue
    {
        std::lock_guard<std::mutex> queueLock(mQueueMutex);
        while (!mDataQueue.empty()) {
            mDataQueue.pop();
        }
        mQueueSize.store(0);
    }
    
    std::cout << "[INFO] Disconnected successfully\n";
}

bool TcpCommunicatorImpl::send_data(const std::vector<uint8_t>& data) {
    if (!mIsConnected.load()) {
        std::cout << "[WARN] Not connected, cannot send data\n";
        return false;
    }
    
    if (data.empty()) {
        std::cout << "[WARN] Empty data, ignoring\n";
        return false;
    }
    
    // Add data to queue for async processing
    enqueue_data(data);
    return true;
}

void TcpCommunicatorImpl::register_global_callback(ResponseCallback cb) {
    std::lock_guard<std::mutex> lock(mCallbackMutex);
    mGlobalCallback = std::move(cb);
}

bool TcpCommunicatorImpl::is_connected() const {
    return mIsConnected.load();
}

void TcpCommunicatorImpl::update() {
    if (!mIsConnected.load()) {
        return;
    }
    
    // Process queued data
    process_data_queue();
    
    // Update connection health
    update_connection_health();
    
    // Handle connection errors if needed
    if (!mIsHealthy.load()) {
        handle_connection_error();
    }
}

ConnectionResult TcpCommunicatorImpl::create_socket() {
    mSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (mSocketFd < 0) {
        std::cout << "[ERROR] Failed to create socket: " << strerror(errno) << "\n";
        return ConnectionResult::SOCKET_ERROR;
    }
    
    return ConnectionResult::SUCCESS;
}

void TcpCommunicatorImpl::close_socket() {
    if (mSocketFd >= 0) {
        // Graceful shutdown
        if (shutdown(mSocketFd, SHUT_RDWR) < 0) {
            std::cout << "[WARN] Shutdown failed: " << strerror(errno) << "\n";
        }
        
        // force shutdown
        if (close(mSocketFd) < 0) {
            std::cout << "[WARN] Close failed: " << strerror(errno) << "\n";
        }
        
        mSocketFd = -1;
    }
}

bool TcpCommunicatorImpl::configure_socket() {
    if (mSocketFd < 0) {
        return false;
    }
    
    // Set socket to non-blocking mode
    int flags = fcntl(mSocketFd, F_GETFL, 0);
    if (flags < 0 || fcntl(mSocketFd, F_SETFL, flags | O_NONBLOCK) < 0) {
        std::cout << "[ERROR] Failed to set non-blocking mode: " << strerror(errno) << "\n";
        return false;
    }
    
    // Enable TCP keepalive if configured
    if (mConfig.enable_keepalive) {
        int enable = 1;
        if (setsockopt(mSocketFd, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable)) < 0) {
            std::cout << "[WARN] Failed to enable keepalive: " << strerror(errno) << "\n";
        } else {
            // Set keepalive parameters
            int idle = mConfig.keepalive_idle;
            int interval = mConfig.keepalive_interval;
            int count = mConfig.keepalive_count;
            
            setsockopt(mSocketFd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));
            setsockopt(mSocketFd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
            setsockopt(mSocketFd, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count));
        }
    }
    
    // Disable Nagle's algorithm for low latency
    int enable = 1;
    if (setsockopt(mSocketFd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable)) < 0) {
        std::cout << "[WARN] Failed to disable Nagle's algorithm: " << strerror(errno) << "\n";
    }
    
    return true;
}

ConnectionResult TcpCommunicatorImpl::perform_connect(const std::string& ip, int32_t port) {
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);
    
    // Attempt non-blocking connect
    int result = ::connect(mSocketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    
    if (result == 0) {
        // Connected immediately (unlikely with non-blocking socket)
        return ConnectionResult::SUCCESS;
    }
    
    if (errno != EINPROGRESS) {
        std::cout << "[ERROR] Connect failed: " << strerror(errno) << "\n";
        
        switch (errno) {
            case ECONNREFUSED:
                return ConnectionResult::CONNECTION_REFUSED;
            case ENETUNREACH:
            case EHOSTUNREACH:
                return ConnectionResult::NETWORK_ERROR;
            default:
                return ConnectionResult::SOCKET_ERROR;
        }
    }
    
    // Wait for connection to complete with timeout
    if (!wait_for_socket_ready(POLLOUT, mConfig.connect_timeout_ms)) {
        std::cout << "[ERROR] Connection timeout after " << mConfig.connect_timeout_ms << "ms\n";
        return ConnectionResult::TIMEOUT;
    }
    
    // Check if connection was successful
    int error = 0;
    socklen_t len = sizeof(error);
    if (getsockopt(mSocketFd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        std::cout << "[ERROR] Failed to get socket error: " << strerror(errno) << "\n";
        return ConnectionResult::SOCKET_ERROR;
    }
    
    if (error != 0) {
        std::cout << "[ERROR] Connection failed: " << strerror(error) << "\n";
        
        switch (error) {
            case ECONNREFUSED:
                return ConnectionResult::CONNECTION_REFUSED;
            case ENETUNREACH:
            case EHOSTUNREACH:
                return ConnectionResult::NETWORK_ERROR;
            case ETIMEDOUT:
                return ConnectionResult::TIMEOUT;
            default:
                return ConnectionResult::SOCKET_ERROR;
        }
    }
    
    return ConnectionResult::SUCCESS;
}

bool TcpCommunicatorImpl::wait_for_socket_ready(int32_t events, uint32_t timeout_ms) {
    struct pollfd pfd;
    pfd.fd = mSocketFd;
    pfd.events = events;
    pfd.revents = 0;
    
    int result = poll(&pfd, 1, timeout_ms);
    
    if (result < 0) {
        std::cout << "[ERROR] Poll failed: " << strerror(errno) << "\n";
        return false;
    }
    
    if (result == 0) {
        // Timeout
        return false;
    }
    
    if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
        std::cout << "[ERROR] Socket error in poll\n";
        return false;
    }
    
    return true;
}

void TcpCommunicatorImpl::enqueue_data(const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(mQueueMutex);
    
    // Prevent queue from growing too large
    const size_t MAX_QUEUE_SIZE = 1000;
    if (mDataQueue.size() >= MAX_QUEUE_SIZE) {
        std::cout << "[WARN] Data queue full, dropping oldest data\n";
        mDataQueue.pop();
    }
    
    mDataQueue.push(data);
    mQueueSize.store(mDataQueue.size());
}

bool TcpCommunicatorImpl::dequeue_data(std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(mQueueMutex);
    
    if (mDataQueue.empty()) {
        return false;
    }
    
    data = mDataQueue.front();
    mDataQueue.pop();
    mQueueSize.store(mDataQueue.size());
    return true;
}

void TcpCommunicatorImpl::process_data_queue() {
    std::vector<uint8_t> data;
    while (dequeue_data(data)) {
        if (!send_data_async(data)) {
            std::cout << "[ERROR] Failed to send data (" << data.size() << " bytes)\n";
            
            // Mark connection as unhealthy
            mIsHealthy.store(false);
            mConnectionErrors.fetch_add(1);
            break;
        }
    }
}

bool TcpCommunicatorImpl::send_data_async(const std::vector<uint8_t>& data) {
    if (mSocketFd < 0 || !mIsConnected.load()) {
        return false;
    }
    
    const uint8_t* buffer = data.data();
    size_t total_sent = 0;
    size_t data_length = data.size();
    
    while (total_sent < data_length) {
        // Wait for socket to be ready for writing
        if (!wait_for_socket_ready(POLLOUT, mConfig.read_timeout_ms)) {
            std::cout << "[ERROR] Send timeout\n";
            return false;
        }
        
        ssize_t sent = send(mSocketFd, buffer + total_sent, data_length - total_sent, MSG_NOSIGNAL);
        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Try again later
                continue;
            }
            
            std::cout << "[ERROR] Send failed: " << strerror(errno) << "\n";
            return false;
        }
        
        total_sent += sent;
        mBytesSent.fetch_add(sent);
    }
    
    return true;
}

void TcpCommunicatorImpl::start_receive_thread() {
    stop_receive_thread(); // Ensure no existing thread
    
    mReceiveEnabled.store(true);
    mReceiveThread = std::make_unique<std::thread>(
        &TcpCommunicatorImpl::receive_thread_function, this);
}

void TcpCommunicatorImpl::stop_receive_thread() {
    mReceiveEnabled.store(false);
    
    if (mReceiveThread && mReceiveThread->joinable()) {
        mReceiveThread->join();
        mReceiveThread.reset();
    }
}

void TcpCommunicatorImpl::receive_thread_function() {
    const size_t BUFFER_SIZE = 4096;
    uint8_t buffer[BUFFER_SIZE];
    std::vector<uint8_t> accumulated_data;
    
    while (mReceiveEnabled.load() && mIsConnected.load()) {
        // Wait for data to be available
        if (!wait_for_socket_ready(POLLIN, 100)) { // 100ms timeout for responsiveness
            continue;
        }
        
        ssize_t received = recv(mSocketFd, buffer, BUFFER_SIZE, MSG_DONTWAIT);
        if (received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            
            std::cout << "[ERROR] Receive failed: " << strerror(errno) << "\n";
            mIsHealthy.store(false);
            break;
        } else if (received == 0) {
            // Connection closed by peer
            std::cout << "[INFO] Connection closed by peer\n";
            mIsConnected.store(false);
            mIsHealthy.store(false);
            break;
        }
        
        // Append received data to accumulated buffer
        accumulated_data.insert(accumulated_data.end(), buffer, buffer + received);
        mBytesReceived.fetch_add(received);
        mLastDataReceived = std::chrono::steady_clock::now();
        
        // Extract complete messages using configured boundary detection
        auto messages = extract_messages_from_buffer(accumulated_data);
        
        // Process each complete message
        for (const auto& message_data : messages) {
            // Call callback if registered
            {
                std::lock_guard<std::mutex> lock(mCallbackMutex);
                if (mGlobalCallback) {
                    mGlobalCallback(message_data);
                }
            }
        }
    }
}

void TcpCommunicatorImpl::start_heartbeat_thread() {
    stop_heartbeat_thread(); // Ensure no existing heartbeat thread
    
    mHeartbeatEnabled.store(true);
    mHeartbeatThread = std::make_unique<std::thread>(
        &TcpCommunicatorImpl::heartbeat_thread_function, this);
}

void TcpCommunicatorImpl::stop_heartbeat_thread() {
    mHeartbeatEnabled.store(false);
    
    if (mHeartbeatThread && mHeartbeatThread->joinable()) {
        mHeartbeatThread->join();
        mHeartbeatThread.reset();
    }
}

void TcpCommunicatorImpl::heartbeat_thread_function() {
    while (mHeartbeatEnabled.load() && mIsConnected.load()) {
        // Use configured heartbeat interval
        std::this_thread::sleep_for(std::chrono::milliseconds(mConfig.heartbeat_interval_ms));
        
        if (!mIsConnected.load()) {
            break;
        }
        
        if (!send_heartbeat()) {
            std::cout << "[WARN] Heartbeat failed for strategy " << 
                mConfig.heartbeat_strategy->get_strategy_name() << "\n";
            mIsHealthy.store(false);
        } else {
            // Update last heartbeat time on successful heartbeat
            mLastHeartbeat = std::chrono::steady_clock::now();
        }
    }
}

bool TcpCommunicatorImpl::send_heartbeat() {
    if (!mConfig.heartbeat_strategy) {
        // No strategy configured, always return true (disabled behavior)
        return true;
    }
    
    return mConfig.heartbeat_strategy->execute_heartbeat(mSocketFd);
}

void TcpCommunicatorImpl::set_connection_config(const ConnectionConfig& config) {
    std::lock_guard<std::mutex> lock(mMutex);
    mConfig = config;
}

ConnectionResult TcpCommunicatorImpl::get_last_connection_result() const {
    return mLastConnectionResult;
}

bool TcpCommunicatorImpl::is_connection_healthy() const {
    return mIsHealthy.load();
}


void TcpCommunicatorImpl::handle_connection_error() {
    std::cout << "[WARN] Handling connection error, attempting reconnection\n";
    
    disconnect_from_server();
    
    // Attempt to reconnect
    if (!mServerIp.empty() && mServerPort > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(mConfig.retry_interval_ms));
        connect_to_server(mServerIp, mServerPort);
    }
}

void TcpCommunicatorImpl::update_connection_health() {
    if (!mIsConnected.load()) {
        return;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto time_since_last_data = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - mLastDataReceived).count();
    
    // Consider connection unhealthy if no data received for too long
    const uint32_t MAX_IDLE_TIME_MS = 60000; // 60 seconds
    if (time_since_last_data > MAX_IDLE_TIME_MS) {
        std::cout << "[WARN] Connection idle for " << time_since_last_data << "ms, marking as unhealthy\n";
        mIsHealthy.store(false);
    }
}

void TcpCommunicatorImpl::register_error_callback(ErrorCallback cb) {
    // This is a placeholder implementation
    (void)cb;
}

// Message boundary detection implementations
std::vector<std::vector<uint8_t>> TcpCommunicatorImpl::extract_messages_from_buffer(std::vector<uint8_t>& buffer) {
    if (!mConfig.message_boundary.detector) {
        std::cout << "[ERROR] No message boundary detector configured\n";
        return {};
    }
    
    return mConfig.message_boundary.detector->extract_messages(buffer);
}
