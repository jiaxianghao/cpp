#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

class ThreadPool {
public:
    explicit ThreadPool(std::size_t thread_count = std::thread::hardware_concurrency())
        : accepting_tasks_(true), stopping_(false), active_tasks_(0) {
        if (thread_count == 0) {
            thread_count = 1;
        }

        workers_.reserve(thread_count);
        try {
            for (std::size_t i = 0; i < thread_count; ++i) {
                workers_.emplace_back([this] { workerLoop(); });
            }
        } catch (...) {
            shutdownNow();
            throw;
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ~ThreadPool() {
        shutdown();
    }

    template <typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>> {
        using Result = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;

        auto task = std::make_shared<std::packaged_task<Result()>>(
            [func = std::forward<F>(f),
             captured_args = std::make_tuple(std::forward<Args>(args)...)]() mutable -> Result {
                return std::apply(
                    [&func](auto&&... unpacked_args) mutable -> Result {
                        if constexpr (std::is_void_v<Result>) {
                            std::invoke(std::move(func), std::forward<decltype(unpacked_args)>(unpacked_args)...);
                        } else {
                            return std::invoke(std::move(func), std::forward<decltype(unpacked_args)>(unpacked_args)...);
                        }
                    },
                    std::move(captured_args));
            });

        std::future<Result> future = task->get_future();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!accepting_tasks_) {
                throw std::runtime_error("cannot enqueue task after thread pool shutdown");
            }
            tasks_.emplace([task] { (*task)(); });
        }

        task_available_.notify_one();
        return future;
    }

    void shutdown() noexcept {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            accepting_tasks_ = false;
            stopping_ = true;
        }

        task_available_.notify_all();
        joinWorkers();
    }

    void shutdownNow() noexcept {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            accepting_tasks_ = false;
            stopping_ = true;
            std::queue<std::function<void()>> empty;
            tasks_.swap(empty);
        }

        task_available_.notify_all();
        joinWorkers();
    }

    void waitIdle() {
        std::unique_lock<std::mutex> lock(mutex_);
        idle_.wait(lock, [this] {
            return tasks_.empty() && active_tasks_ == 0;
        });
    }

    std::size_t threadCount() const noexcept {
        return workers_.size();
    }

private:
    void workerLoop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                task_available_.wait(lock, [this] {
                    return stopping_ || !tasks_.empty();
                });

                if (stopping_ && tasks_.empty()) {
                    return;
                }

                task = std::move(tasks_.front());
                tasks_.pop();
                ++active_tasks_;
            }

            try {
                task();
            } catch (...) {
                // packaged_task stores exceptions in the future; this is a guard
                // for tasks submitted through future internal extensions.
            }

            {
                std::lock_guard<std::mutex> lock(mutex_);
                --active_tasks_;
                if (tasks_.empty() && active_tasks_ == 0) {
                    idle_.notify_all();
                }
            }
        }
    }

    void joinWorkers() noexcept {
        for (std::thread& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    mutable std::mutex mutex_;
    std::condition_variable task_available_;
    std::condition_variable idle_;
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    bool accepting_tasks_;
    bool stopping_;
    std::size_t active_tasks_;
};
