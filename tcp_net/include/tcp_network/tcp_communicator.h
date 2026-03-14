#pragma once

#include "tcp_communicator_interface.h"
#include "tcp_communicator_config.h"
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <chrono>

namespace tcp_network {

// Enhanced SocketTcpCommunicator implementation
class TcpCommunicatorImpl : public ITcpCommunicator {
public:
    TcpCommunicatorImpl();
    ~TcpCommunicatorImpl() override;

    // ITcpCommunicator interface implementation
    bool connect_to_server(const std::string& ip, int32_t port) override;
    void disconnect_from_server() override;
    bool send_data(const std::vector<uint8_t>& data) override;
    void register_global_callback(ResponseCallback cb) override;
    bool is_connected() const override;
    void update() override;

    // Enhanced functionality
    void set_connection_config(const ConnectionConfig& config);
    ConnectionResult get_last_connection_result() const override;
    bool is_connection_healthy() const override;
    void register_error_callback(ErrorCallback cb);

private:
    // Core connection methods
    ConnectionResult create_socket();
    void close_socket();
    bool configure_socket();
    ConnectionResult perform_connect(const std::string& ip, int32_t port);
    
    // Async I/O methods
    bool send_data_async(const std::vector<uint8_t>& data);
    bool wait_for_socket_ready(int32_t events, uint32_t timeout_ms);
    
    // Thread management
    void start_receive_thread();
    void stop_receive_thread();
    void start_heartbeat_thread();
    void stop_heartbeat_thread();
    void receive_thread_function();
    void heartbeat_thread_function();
    
    // Connection health
    bool send_heartbeat();
    void handle_connection_error();
    void update_connection_health();

    // Thread-safe queue operations
    void enqueue_data(const std::vector<uint8_t>& data);
    bool dequeue_data(std::vector<uint8_t>& data);
    void process_data_queue();
    
    // Message boundary detection methods
    std::vector<std::vector<uint8_t>> extract_messages_from_buffer(std::vector<uint8_t>& buffer);

private:
    // Socket and connection state
    int32_t mSocketFd;
    std::atomic<bool> mIsConnected;
    std::atomic<bool> mIsHealthy;
    ConnectionResult mLastConnectionResult;
    
    // Connection configuration
    ConnectionConfig mConfig;
    std::string mServerIp;
    int32_t mServerPort;
    
    // Thread safety
    mutable std::mutex mMutex;
    mutable std::mutex mQueueMutex;
    
    // Data queue
    std::queue<std::vector<uint8_t>> mDataQueue;
    std::atomic<size_t> mQueueSize;
    
    // Callback handling
    ResponseCallback mGlobalCallback;
    std::mutex mCallbackMutex;
    
    // Threading
    std::unique_ptr<std::thread> mReceiveThread;
    std::unique_ptr<std::thread> mHeartbeatThread;
    std::atomic<bool> mReceiveEnabled;
    
    // Connection health monitoring
    std::chrono::steady_clock::time_point mLastHeartbeat;
    std::chrono::steady_clock::time_point mLastDataReceived;
    std::atomic<bool> mHeartbeatEnabled;
    
    // Statistics
    std::atomic<uint64_t> mBytesSent;
    std::atomic<uint64_t> mBytesReceived;
    std::atomic<uint32_t> mConnectionErrors;
};

} // namespace tcp_network