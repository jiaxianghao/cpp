#pragma once

#include "connection.h"
#include "event_loop.h"

#include <string>
#include <unordered_map>
#include <cstdint>

// TCP 监听服务器：接受连接，分发事件
class TcpServer {
public:
    using MessageCallback = TcpConnection::MessageCallback;

    TcpServer(EventLoop* loop, const std::string& host, uint16_t port);
    ~TcpServer();

    void set_message_cb(MessageCallback cb) { message_cb_ = std::move(cb); }

    // 开始监听并注册 accept 事件
    void start();

private:
    void on_accept(uint32_t events);
    void on_conn_close(TcpConnectionPtr conn);

    EventLoop*  loop_;
    int         listen_fd_{-1};
    int         next_id_{0};
    std::string host_;
    uint16_t    port_;

    std::unordered_map<int /*conn_id*/, TcpConnectionPtr> conns_;
    MessageCallback message_cb_;
};
