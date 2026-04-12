#include "connection.h"

#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>

// ─────────────────────────────────────────────
// 构造 / 析构
// ─────────────────────────────────────────────

TcpConnection::TcpConnection(EventLoop* loop, int sockfd, int id)
    : loop_(loop), fd_(sockfd), id_(id) {}

TcpConnection::~TcpConnection() {
    // 防御性清理：正常路径已在 do_close() 中完成
    if (registered_) {
        loop_->remove(fd_);
        registered_ = false;
    }
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

// ─────────────────────────────────────────────
// 注册到事件循环
// ─────────────────────────────────────────────

void TcpConnection::start() {
    // 用 weak_ptr 捕获，避免 EventLoop 的 callbacks_ 持有 shared_ptr 造成泄漏
    auto weak = weak_from_this();
    loop_->add(fd_, interest_, [weak](uint32_t ev) {
        if (auto self = weak.lock())
            self->on_event(ev);
    });
    registered_ = true;
}

// ─────────────────────────────────────────────
// epoll 事件分发入口
// ─────────────────────────────────────────────

void TcpConnection::on_event(uint32_t events) {
    // 持有自身，防止回调内部触发析构
    auto guard = shared_from_this();

    if (events & (EPOLLERR | EPOLLHUP)) {
        std::cerr << "[conn " << id_ << "] error/hangup\n";
        do_close();
        return;
    }

    // EPOLLRDHUP 也先做一次 read，确保把剩余数据取走再关闭
    if (!closed_ && (events & (EPOLLIN | EPOLLRDHUP))) do_read();
    if (!closed_ && (events & EPOLLOUT))               do_write();
}

// ─────────────────────────────────────────────
// 非阻塞读：ET 模式下循环读到 EAGAIN
// ─────────────────────────────────────────────

void TcpConnection::do_read() {
    bool peer_closed = false;

    while (true) {
        read_buf_.ensure_writable(4096);
        ssize_t n = ::read(fd_, read_buf_.write_ptr(), read_buf_.writable());

        if (n > 0) {
            read_buf_.advance_write(static_cast<size_t>(n));
        } else if (n == 0) {
            // EOF：先记录标志，把已读数据派发后再关闭
            peer_closed = true;
            break;
        } else {
            if (errno == EINTR)                        continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) break; // ET 模式：暂无更多数据
            std::cerr << "[conn " << id_ << "] read error: " << strerror(errno) << '\n';
            do_close();
            return;
        }
    }

    // 先把缓冲区数据派发给上层，再处理关闭
    if (!closed_ && !read_buf_.empty() && message_cb_)
        message_cb_(shared_from_this(), read_buf_);

    if (!closed_ && peer_closed)
        do_close();
}

// ─────────────────────────────────────────────
// 异步发送：优先直接写，写不完则入队并等待 EPOLLOUT
// ─────────────────────────────────────────────

void TcpConnection::send(const void* data, size_t len) {
    if (closed_ || len == 0) return;

    const char* ptr       = static_cast<const char*>(data);
    size_t      remaining = len;

    // 只有写缓冲区为空时才尝试直接写（保证顺序）
    if (write_buf_.empty()) {
        while (remaining > 0) {
            ssize_t n = ::write(fd_, ptr, remaining);
            if (n > 0) {
                ptr       += n;
                remaining -= static_cast<size_t>(n);
            } else if (n == -1) {
                if (errno == EINTR)                        continue;
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                std::cerr << "[conn " << id_ << "] write error: " << strerror(errno) << '\n';
                do_close();
                return;
            }
        }
    }

    // 将未发送的数据缓入写缓冲区，注册 EPOLLOUT
    if (remaining > 0) {
        write_buf_.append(ptr, remaining);
        if (!(interest_ & EPOLLOUT)) {
            interest_ |= EPOLLOUT;
            update_interest();
        }
    }
}

// ─────────────────────────────────────────────
// EPOLLOUT 触发：继续发送缓冲数据
// ─────────────────────────────────────────────

void TcpConnection::do_write() {
    while (!write_buf_.empty()) {
        ssize_t n = ::write(fd_, write_buf_.read_ptr(), write_buf_.readable());
        if (n > 0) {
            write_buf_.consume(static_cast<size_t>(n));
        } else if (n == -1) {
            if (errno == EINTR)                        continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            std::cerr << "[conn " << id_ << "] write error: " << strerror(errno) << '\n';
            do_close();
            return;
        }
    }

    // 发送完毕，取消 EPOLLOUT 监听
    if (write_buf_.empty() && (interest_ & EPOLLOUT)) {
        interest_ &= ~static_cast<uint32_t>(EPOLLOUT);
        update_interest();
    }
}

// ─────────────────────────────────────────────
// 关闭连接
// ─────────────────────────────────────────────

void TcpConnection::do_close() {
    if (closed_) return;
    closed_ = true;

    std::cout << "[conn " << id_ << "] closed (fd=" << fd_ << ")\n";

    if (registered_) {
        loop_->remove(fd_);
        registered_ = false;
    }
    ::close(fd_);
    fd_ = -1;

    if (close_cb_)
        close_cb_(shared_from_this());
}

// ─────────────────────────────────────────────
// 同步 interest_ 到 epoll（复用 weak_ptr 回调）
// ─────────────────────────────────────────────

void TcpConnection::update_interest() {
    if (!registered_) return;
    auto weak = weak_from_this();
    loop_->modify(fd_, interest_, [weak](uint32_t ev) {
        if (auto self = weak.lock())
            self->on_event(ev);
    });
}
