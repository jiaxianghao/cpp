#include "thread_pool.hpp"

#include <atomic>
#include <chrono>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void runsSubmittedTasks() {
    ThreadPool pool(3);
    std::vector<std::future<int>> futures;

    for (int i = 0; i < 20; ++i) {
        futures.emplace_back(pool.enqueue([i] {
            return i + 1;
        }));
    }

    int sum = 0;
    for (auto& future : futures) {
        sum += future.get();
    }

    require(sum == 210, "submitted tasks returned incorrect results");
}

void supportsVoidTasksAndWaitIdle() {
    ThreadPool pool(2);
    std::atomic<int> counter{0};

    for (int i = 0; i < 50; ++i) {
        pool.enqueue([&counter] {
            counter.fetch_add(1, std::memory_order_relaxed);
        });
    }

    pool.waitIdle();
    require(counter.load(std::memory_order_relaxed) == 50, "waitIdle returned before tasks completed");
}

void propagatesTaskExceptions() {
    ThreadPool pool(1);
    auto future = pool.enqueue([]() -> int {
        throw std::runtime_error("expected failure");
    });

    bool threw = false;
    try {
        (void)future.get();
    } catch (const std::runtime_error& error) {
        threw = std::string(error.what()) == "expected failure";
    }

    require(threw, "task exception was not propagated through future");
}

void rejectsTasksAfterShutdown() {
    ThreadPool pool(1);
    pool.shutdown();

    bool threw = false;
    try {
        (void)pool.enqueue([] { return 1; });
    } catch (const std::runtime_error&) {
        threw = true;
    }

    require(threw, "enqueue accepted task after shutdown");
}

}  // namespace

int main() {
    runsSubmittedTasks();
    supportsVoidTasksAndWaitIdle();
    propagatesTaskExceptions();
    rejectsTasksAfterShutdown();
    return 0;
}
