/*
 * Simple TCP Communicator Example
 * 展示TCP通信库的基本用法
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <tcp_network/tcp_communicator.h>
#include <tcp_network/heartbeat_strategy.h>

using namespace tcp_network;

class SimpleTcpClient {
public:
    SimpleTcpClient() {
        // 配置连接参数
        ConnectionConfig config;
        config.connect_timeout_ms = 5000;    // 5秒连接超时
        config.read_timeout_ms = 3000;       // 3秒读取超时
        config.retry_count = 3;              // 重试3次
        config.retry_interval_ms = 1000;     // 重试间隔1秒
        config.enable_keepalive = true;      // 启用TCP keepalive
        
        // 配置消息边界检测 - 使用换行符分隔
        config.message_boundary = MessageBoundaryConfig::newline_delimited();
        
        // 配置心跳策略 - 使用socket状态检查
        config.heartbeat_strategy = create_heartbeat_strategy(HeartbeatType::SOCKET_STATUS);
        config.heartbeat_interval_ms = 3000; // 3秒检查一次
        
        // 创建通信器
        mCommunicator = std::make_unique<TcpCommunicatorImpl>();
        mCommunicator->set_connection_config(config);
        
        // 注册响应回调
        mCommunicator->register_global_callback(
            [this](const std::vector<uint8_t>& response) {
                handle_response(response);
            });
    }
    
    ~SimpleTcpClient() {
        disconnect();
    }
    
    // 连接到服务器
    bool connect_to_server(const std::string& ip, int port) {
        std::cout << "正在连接到 " << ip << ":" << port << "..." << std::endl;
        
        bool result = mCommunicator->connect_to_server(ip, port);
        
        if (result) {
            std::cout << "✓ 连接成功!" << std::endl;
        } else {
            std::cout << "✗ 连接失败: " << get_error_string() << std::endl;
        }
        
        return result;
    }
    
    // 断开连接
    void disconnect() {
        if (mCommunicator && mCommunicator->is_connected()) {
            std::cout << "正在断开连接..." << std::endl;
            mCommunicator->disconnect_from_server();
            std::cout << "✓ 已断开连接" << std::endl;
        }
    }
    
    // 发送消息
    bool send_message(const std::string& message) {
        if (!mCommunicator->is_connected()) {
            std::cout << "✗ 未连接到服务器" << std::endl;
            return false;
        }
        
        // 将字符串转换为字节数组，并添加换行符
        std::vector<uint8_t> data(message.begin(), message.end());
        data.push_back('\n'); // 添加换行符作为消息分隔符
        
        std::cout << "发送: " << message << std::endl;
        return mCommunicator->send_data(data);
    }
    
    // 检查连接状态
    bool is_connected() const {
        return mCommunicator->is_connected();
    }
    
    // 检查连接健康状态
    bool is_healthy() const {
        return mCommunicator->is_connection_healthy();
    }
    
    // 更新连接状态（需要在主循环中调用）
    void update() {
        if (mCommunicator) {
            mCommunicator->update();
        }
    }
    
private:
    std::unique_ptr<TcpCommunicatorImpl> mCommunicator;
    
    // 处理服务器响应
    void handle_response(const std::vector<uint8_t>& response) {
        // 将字节数组转换为字符串
        std::string response_str(response.begin(), response.end());
        std::cout << "收到响应: " << response_str << std::endl;
    }
    
    // 获取错误信息
    std::string get_error_string() const {
        if (!mCommunicator) {
            return "通信器未初始化";
        }
        
        ConnectionResult result = mCommunicator->get_last_connection_result();
        switch (result) {
            case ConnectionResult::SUCCESS:
                return "成功";
            case ConnectionResult::INVALID_ADDRESS:
                return "无效的IP地址";
            case ConnectionResult::CONNECTION_REFUSED:
                return "连接被拒绝";
            case ConnectionResult::TIMEOUT:
                return "连接超时";
            case ConnectionResult::NETWORK_ERROR:
                return "网络错误";
            case ConnectionResult::SOCKET_ERROR:
                return "Socket错误";
            case ConnectionResult::ALREADY_CONNECTED:
                return "已经连接";
            default:
                return "未知错误";
        }
    }
};

// 简单的Echo服务器实现
class SimpleEchoServer {
public:
    SimpleEchoServer(int port) : mPort(port), mRunning(false), mServerSocket(-1) {}
    
    ~SimpleEchoServer() {
        stop();
    }
    
    bool start() {
        // 创建服务器socket
        mServerSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (mServerSocket < 0) {
            std::cout << "✗ 创建服务器socket失败" << std::endl;
            return false;
        }
        
        // 设置socket选项，允许地址重用
        int opt = 1;
        if (setsockopt(mServerSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cout << "✗ 设置socket选项失败" << std::endl;
            close(mServerSocket);
            return false;
        }
        
        // 绑定地址
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(mPort);
        
        if (bind(mServerSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cout << "✗ 绑定端口 " << mPort << " 失败" << std::endl;
            close(mServerSocket);
            return false;
        }
        
        // 开始监听
        if (listen(mServerSocket, 5) < 0) {
            std::cout << "✗ 监听失败" << std::endl;
            close(mServerSocket);
            return false;
        }
        
        mRunning = true;
        mServerThread = std::thread([this]() {
            run_server();
        });
        
        std::cout << "✓ Echo服务器启动在端口 " << mPort << std::endl;
        return true;
    }
    
    void stop() {
        mRunning = false;
        if (mServerSocket >= 0) {
            close(mServerSocket);
            mServerSocket = -1;
        }
        if (mServerThread.joinable()) {
            mServerThread.join();
        }
        std::cout << "✓ Echo服务器已停止" << std::endl;
    }
    
private:
    int mPort;
    bool mRunning;
    int mServerSocket;
    std::thread mServerThread;
    
    void run_server() {
        std::cout << "Echo服务器正在等待连接..." << std::endl;
        
        while (mRunning) {
            struct sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            
            // 接受连接
            int clientSocket = accept(mServerSocket, (struct sockaddr*)&clientAddr, &clientLen);
            if (clientSocket < 0) {
                if (mRunning) {
                    std::cout << "✗ 接受连接失败" << std::endl;
                }
                continue;
            }
            
            // 获取客户端IP地址
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            std::cout << "✓ 客户端连接: " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;
            
            // 处理客户端连接
            handle_client(clientSocket);
            
            close(clientSocket);
            std::cout << "客户端断开连接" << std::endl;
        }
    }
    
    void handle_client(int clientSocket) {
        char buffer[1024];
        
        while (mRunning) {
            // 接收数据
            ssize_t received = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (received <= 0) {
                break; // 连接断开或错误
            }
            
            buffer[received] = '\0';
            std::string message(buffer);
            
            // 移除换行符
            if (!message.empty() && message.back() == '\n') {
                message.pop_back();
            }
            
            std::cout << "服务器收到: " << message << std::endl;
            
            // 构造响应（添加"Echo: "前缀）
            std::string response = "Echo: " + message + "\n";
            
            // 发送响应
            ssize_t sent = send(clientSocket, response.c_str(), response.length(), 0);
            if (sent < 0) {
                std::cout << "✗ 发送响应失败" << std::endl;
                break;
            }
            
            std::cout << "服务器发送: " << response.substr(0, response.length() - 1) << std::endl;
        }
    }
};

int main() {
    std::cout << "=== TCP通信库简单示例 ===" << std::endl;
    
    // 服务器配置
    std::string server_ip = "127.0.0.1";
    int server_port = 8080;
    
    // 1. 启动Echo服务器
    std::cout << "\n1. 启动Echo服务器..." << std::endl;
    SimpleEchoServer server(server_port);
    if (!server.start()) {
        std::cout << "✗ 服务器启动失败" << std::endl;
        return 1;
    }
    
    // 等待服务器完全启动
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // 2. 创建TCP客户端并连接
    std::cout << "\n2. 创建TCP客户端..." << std::endl;
    SimpleTcpClient client;
    
    std::cout << "\n3. 连接到服务器..." << std::endl;
    if (client.connect_to_server(server_ip, server_port)) {
        std::cout << "\n4. 发送测试消息..." << std::endl;
        
        // 发送一些测试消息
        std::vector<std::string> messages = {
            "Hello Server!",
            "How are you?",
            "This is a test message",
            "Goodbye!"
        };
        
        for (const auto& msg : messages) {
            if (client.send_message(msg)) {
                // 等待响应
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                
                // 更新连接状态
                client.update();
                
                // 检查连接健康状态
                if (!client.is_healthy()) {
                    std::cout << "⚠ 连接不健康，尝试重连..." << std::endl;
                }
            } else {
                std::cout << "✗ 发送消息失败" << std::endl;
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        std::cout << "\n5. 断开连接..." << std::endl;
        client.disconnect();
    } else {
        std::cout << "\n✗ 无法连接到服务器" << std::endl;
    }
    
    // 6. 停止服务器
    std::cout << "\n6. 停止服务器..." << std::endl;
    server.stop();
    
    std::cout << "\n=== 示例结束 ===" << std::endl;
    return 0;
}
