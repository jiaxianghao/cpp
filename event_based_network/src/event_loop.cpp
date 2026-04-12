#include "event_loop.h"

#include <sys/signalfd.h>
#include <unistd.h>
#include <csignal>
#include <cstring>
#include <stdexcept>
#include <iostream>

// ─────────────────────────────────────────────
// 构造 / 析构
// ─────────────────────────────────────────────

EventLoop::EventLoop() {
    epoll_fd_ = ::epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd_ == -1)
        throw std::runtime_error(std::string("epoll_create1: ") + strerror(errno));

    setup_signals();
}

EventLoop::~EventLoop() {
    if (signal_fd_ >= 0) ::close(signal_fd_);
    if (epoll_fd_ >= 0)  ::close(epoll_fd_);
}

// ─────────────────────────────────────────────
// 信号处理：使用 signalfd 将信号纳入 epoll 统一分发
// ─────────────────────────────────────────────

void EventLoop::setup_signals() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);

    // 屏蔽信号，防止默认处理
    if (::sigprocmask(SIG_BLOCK, &mask, nullptr) == -1)
        throw std::runtime_error(std::string("sigprocmask: ") + strerror(errno));

    signal_fd_ = ::signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    if (signal_fd_ == -1)
        throw std::runtime_error(std::string("signalfd: ") + strerror(errno));

    add(signal_fd_, EPOLLIN, [this](uint32_t) { on_signal(); });
}

void EventLoop::on_signal() {
    signalfd_siginfo info{};
    ssize_t n = ::read(signal_fd_, &info, sizeof(info));
    if (n != static_cast<ssize_t>(sizeof(info))) return;

    std::cout << "\n[signal] received " << info.ssi_signo
              << " (" << strsignal(info.ssi_signo) << "), stopping...\n";
    quit();
}

// ─────────────────────────────────────────────
// fd 管理
// ─────────────────────────────────────────────

void EventLoop::add(int fd, uint32_t events, Callback cb) {
    callbacks_[fd] = std::move(cb);

    epoll_event ev{};
    ev.events  = events;
    ev.data.fd = fd;

    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        callbacks_.erase(fd);
        throw std::runtime_error(std::string("epoll_ctl ADD fd=")
                                 + std::to_string(fd) + ": " + strerror(errno));
    }
}

void EventLoop::modify(int fd, uint32_t events, Callback cb) {
    callbacks_[fd] = std::move(cb);

    epoll_event ev{};
    ev.events  = events;
    ev.data.fd = fd;

    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == -1)
        throw std::runtime_error(std::string("epoll_ctl MOD fd=")
                                 + std::to_string(fd) + ": " + strerror(errno));
}

void EventLoop::remove(int fd) {
    ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
    callbacks_.erase(fd);
}

// ─────────────────────────────────────────────
// 事件循环主体
// ─────────────────────────────────────────────

void EventLoop::run() {
    running_ = true;
    epoll_event events[kMaxEvents];

    while (running_) {
        int n = ::epoll_wait(epoll_fd_, events, kMaxEvents, -1 /*无超时*/);

        if (n == -1) {
            if (errno == EINTR) continue; // 被信号中断，重试
            throw std::runtime_error(std::string("epoll_wait: ") + strerror(errno));
        }

        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            auto it = callbacks_.find(fd);
            if (it == callbacks_.end()) continue;

            // 复制回调再调用：允许回调内部安全地调用 remove(fd)
            Callback cb = it->second;
            cb(events[i].events);
        }
    }
}

void EventLoop::quit() {
    running_ = false;
}
