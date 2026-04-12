#pragma once

#include <functional>
#include <unordered_map>
#include <sys/epoll.h>

// 基于 epoll(ET) + signalfd 的单线程事件循环
class EventLoop {
public:
    using Callback = std::function<void(uint32_t /*events*/)>;

    EventLoop();
    ~EventLoop();

    // 禁止拷贝
    EventLoop(const EventLoop&)            = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    // 向 epoll 注册/修改/删除 fd
    void add(int fd, uint32_t events, Callback cb);
    void modify(int fd, uint32_t events, Callback cb);
    void remove(int fd);

    // 启动事件循环（阻塞直到 quit() 被调用）
    void run();

    // 请求退出循环
    void quit();

private:
    void setup_signals();   // 通过 signalfd 接管 SIGINT / SIGTERM
    void on_signal();       // 读取并处理信号

    int  epoll_fd_{-1};
    int  signal_fd_{-1};
    bool running_{false};

    std::unordered_map<int, Callback> callbacks_;

    static constexpr int kMaxEvents = 1024;
};
