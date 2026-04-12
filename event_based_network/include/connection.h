#pragma once

#include "buffer.h"
#include "event_loop.h"

#include <functional>
#include <memory>

class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

// 代表一条 TCP 连接（非阻塞 socket，边缘触发）
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    // 收到数据后的回调；用户从 read_buf 中取走数据
    using MessageCallback = std::function<void(TcpConnectionPtr, Buffer& /*read_buf*/)>;
    // 连接关闭时的回调
    using CloseCallback   = std::function<void(TcpConnectionPtr)>;

    TcpConnection(EventLoop* loop, int sockfd, int id);
    ~TcpConnection();

    void set_message_cb(MessageCallback cb) { message_cb_ = std::move(cb); }
    void set_close_cb(CloseCallback cb)     { close_cb_   = std::move(cb); }

    // 注册到事件循环，开始监听
    void start();

    // 异步发送数据
    void send(const void* data, size_t len);
    void send(const std::string& s) { send(s.data(), s.size()); }

    int id() const { return id_; }
    int fd() const { return fd_; }

private:
    void on_event(uint32_t events);  // epoll 回调入口
    void do_read();
    void do_write();
    void do_close();
    void update_interest();          // 同步 interest_ 到 epoll

    EventLoop* loop_;
    int        fd_;
    int        id_;
    bool       closed_{false};
    bool       registered_{false};

    // epoll 关注的事件集合（始终含 EPOLLET）
    uint32_t interest_{EPOLLIN | EPOLLRDHUP | EPOLLET};

    Buffer read_buf_;
    Buffer write_buf_;

    MessageCallback message_cb_;
    CloseCallback   close_cb_;
};
