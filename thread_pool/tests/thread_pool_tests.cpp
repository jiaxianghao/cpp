#include "thread_pool.hpp"

#include <atomic>
#include <chrono>
#include <functional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

int add(int lhs, int rhs) {
    return lhs + rhs;
}

struct Accumulator {
    explicit Accumulator(int base) : base(base) {}

    int addToBase(int value) const {
        return base + value;
    }

    int operator()(int value) const {
        return addToBase(value);
    }

    int base;
};

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

void supportsCommonCallableTypes() {
    ThreadPool pool(2);

    Accumulator accumulator(10);
    int offset = 20;

    auto free_function = pool.enqueue(add, 1, 2);
    auto member_function = pool.enqueue(&Accumulator::addToBase, &accumulator, 3);
    auto member_data = pool.enqueue(&Accumulator::base, &accumulator);
    auto lambda = pool.enqueue([offset](int value) {
        return offset + value;
    }, 4);
    auto function_object = pool.enqueue(accumulator, 5);
    auto reference_wrapper = pool.enqueue(std::ref(accumulator), 6);

    require(free_function.get() == 3, "free function task returned incorrect result");
    require(member_function.get() == 13, "member function task returned incorrect result");
    require(member_data.get() == 10, "member data task returned incorrect result");
    require(lambda.get() == 24, "lambda task returned incorrect result");
    require(function_object.get() == 15, "function object task returned incorrect result");
    require(reference_wrapper.get() == 16, "reference_wrapper task returned incorrect result");
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
    supportsCommonCallableTypes();
    supportsVoidTasksAndWaitIdle();
    propagatesTaskExceptions();
    rejectsTasksAfterShutdown();
    return 0;
}
