#include "server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <iostream>

// ─────────────────────────────────────────────
// 构造 / 析构
// ─────────────────────────────────────────────

TcpServer::TcpServer(EventLoop* loop, const std::string& host, uint16_t port)
    : loop_(loop), host_(host), port_(port) {

    listen_fd_ = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (listen_fd_ == -1)
        throw std::runtime_error(std::string("socket: ") + strerror(errno));

    // 地址复用，方便快速重启
    int opt = 1;
    if (::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error(std::string("setsockopt SO_REUSEADDR: ") + strerror(errno));
    if (::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
        throw std::runtime_error(std::string("setsockopt SO_REUSEPORT: ") + strerror(errno));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port_);
    if (::inet_pton(AF_INET, host_.c_str(), &addr.sin_addr) != 1)
        throw std::runtime_error("inet_pton: invalid address " + host_);

    if (::bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1)
        throw std::runtime_error(std::string("bind: ") + strerror(errno));

    if (::listen(listen_fd_, SOMAXCONN) == -1)
        throw std::runtime_error(std::string("listen: ") + strerror(errno));
}

TcpServer::~TcpServer() {
    if (listen_fd_ >= 0) {
        loop_->remove(listen_fd_);
        ::close(listen_fd_);
    }
}

// ─────────────────────────────────────────────
// 开始监听
// ─────────────────────────────────────────────

void TcpServer::start() {
    std::cout << "[server] listening on " << host_ << ":" << port_ << '\n';
    // accept 使用水平触发（LT）即可，不需要 ET
    loop_->add(listen_fd_, EPOLLIN, [this](uint32_t ev) { on_accept(ev); });
}

// ─────────────────────────────────────────────
// 接受新连接（循环 accept 直到 EAGAIN）
// ─────────────────────────────────────────────

void TcpServer::on_accept(uint32_t /*events*/) {
    while (true) {
        sockaddr_in peer{};
        socklen_t   peer_len = sizeof(peer);

        // accept4 原子设置 NONBLOCK | CLOEXEC
        int conn_fd = ::accept4(listen_fd_,
                                reinterpret_cast<sockaddr*>(&peer),
                                &peer_len,
                                SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (conn_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break; // 不再有待处理连接
            if (errno == EINTR)                          continue;
            std::cerr << "[server] accept4 error: " << strerror(errno) << '\n';
            break;
        }

        char peer_ip[INET_ADDRSTRLEN];
        ::inet_ntop(AF_INET, &peer.sin_addr, peer_ip, sizeof(peer_ip));
        int peer_port = ntohs(peer.sin_port);

        int id = next_id_++;
        std::cout << "[server] new connection id=" << id
                  << " from " << peer_ip << ":" << peer_port
                  << " (fd=" << conn_fd << ")\n";

        auto conn = std::make_shared<TcpConnection>(loop_, conn_fd, id);
        conn->set_message_cb(message_cb_);
        conn->set_close_cb([this](TcpConnectionPtr c) { on_conn_close(c); });

        conns_[id] = conn;
        conn->start();
    }
}

// ─────────────────────────────────────────────
// 连接关闭回调：从 map 中移除
// ─────────────────────────────────────────────

void TcpServer::on_conn_close(TcpConnectionPtr conn) {
    conns_.erase(conn->id());
    std::cout << "[server] connection id=" << conn->id()
              << " removed, active=" << conns_.size() << '\n';
}
