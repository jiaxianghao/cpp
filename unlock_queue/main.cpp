#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

#include "include/mpmc_queue.h"
#include "include/ms_queue.h"
#include "include/spsc_queue.h"

// ──────────────────────────────────────────────────────────────────────────────
// 辅助工具
// ──────────────────────────────────────────────────────────────────────────────

static int g_pass = 0, g_fail = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (cond) {                                                        \
            ++g_pass;                                                      \
        } else {                                                           \
            ++g_fail;                                                      \
            std::cerr << "  [FAIL] " #cond " at line " << __LINE__ << "\n"; \
        }                                                                  \
    } while (0)

static void section(const std::string& title) {
    std::cout << "\n══ " << title << " ══\n";
}

// 纳秒级计时
template <typename F>
static double bench_ns(F&& fn, int iters = 1) {
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < iters; ++i) fn();
    auto t1 = std::chrono::steady_clock::now();
    return static_cast<double>(
               std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()) /
           iters;
}

// ──────────────────────────────────────────────────────────────────────────────
// 1. SPSCQueue 单线程正确性测试
// ──────────────────────────────────────────────────────────────────────────────

static void test_spsc_single_thread() {
    section("SPSCQueue — 单线程正确性");

    SPSCQueue<int, 8> q;
    CHECK(q.empty());
    CHECK(q.size() == 0);

    // 入队直到满
    for (int i = 0; i < 8; ++i)
        CHECK(q.push(i));
    CHECK(!q.push(99));  // 队列已满
    CHECK(q.size() == 8);

    // 出队验证 FIFO 顺序
    for (int i = 0; i < 8; ++i) {
        int val = -1;
        CHECK(q.pop(val));
        CHECK(val == i);
    }
    CHECK(q.empty());

    // 空队列出队
    int dummy;
    CHECK(!q.pop(dummy));

    // 交替 push/pop
    CHECK(q.push(42));
    int v;
    CHECK(q.pop(v));
    CHECK(v == 42);

    std::cout << "  capacity=" << q.capacity() << "\n";
}

// ──────────────────────────────────────────────────────────────────────────────
// 2. SPSCQueue 多线程吞吐量测试
// ──────────────────────────────────────────────────────────────────────────────

static void test_spsc_multi_thread() {
    section("SPSCQueue — 多线程 (1P/1C)");

    constexpr int      N = 2'000'000;
    SPSCQueue<int, 65536> q;
    std::atomic<long long> sum_sent{0}, sum_recv{0};

    auto t0 = std::chrono::steady_clock::now();

    std::thread producer([&] {
        long long s = 0;
        for (int i = 1; i <= N; ++i) {
            while (!q.push(i))
                std::this_thread::yield();
            s += i;
        }
        sum_sent.store(s, std::memory_order_release);
    });

    std::thread consumer([&] {
        long long s = 0;
        int       recv = 0;
        while (recv < N) {
            int val;
            if (q.pop(val)) {
                s += val;
                ++recv;
            } else {
                std::this_thread::yield();
            }
        }
        sum_recv.store(s, std::memory_order_release);
    });

    producer.join();
    consumer.join();

    auto t1 = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    CHECK(sum_sent.load() == sum_recv.load());
    std::cout << std::fixed << std::setprecision(1)
              << "  ops=" << N << "  time=" << ms << "ms"
              << "  throughput=" << N / ms / 1000.0 << " M ops/s\n";
}

// ──────────────────────────────────────────────────────────────────────────────
// 3. MPMCQueue 单线程正确性测试
// ──────────────────────────────────────────────────────────────────────────────

static void test_mpmc_single_thread() {
    section("MPMCQueue — 单线程正确性");

    MPMCQueue<int, 4> q;
    CHECK(q.empty());

    CHECK(q.push(1));
    CHECK(q.push(2));
    CHECK(q.push(3));
    CHECK(q.push(4));
    CHECK(!q.push(5));  // 满

    int v;
    CHECK(q.pop(v) && v == 1);
    CHECK(q.pop(v) && v == 2);
    CHECK(q.pop(v) && v == 3);
    CHECK(q.pop(v) && v == 4);
    CHECK(!q.pop(v));  // 空

    CHECK(q.empty());
    std::cout << "  capacity=" << q.capacity() << "\n";
}

// ──────────────────────────────────────────────────────────────────────────────
// 4. MPMCQueue 多线程正确性 + 吞吐量测试
// ──────────────────────────────────────────────────────────────────────────────

static void test_mpmc_multi_thread() {
    section("MPMCQueue — 多线程 (4P/4C)");

    constexpr int PRODUCERS  = 4;
    constexpr int CONSUMERS  = 4;
    constexpr int PER_THREAD = 500'000;
    constexpr int TOTAL      = PRODUCERS * PER_THREAD;

    MPMCQueue<int, 4096> q;
    std::atomic<long long> sum_sent{0}, sum_recv{0};
    std::atomic<int>       recv_count{0};

    auto t0 = std::chrono::steady_clock::now();

    std::vector<std::thread> producers;
    for (int t = 0; t < PRODUCERS; ++t) {
        producers.emplace_back([&, t] {
            long long s = 0;
            int start = t * PER_THREAD + 1;
            for (int i = start; i < start + PER_THREAD; ++i) {
                while (!q.push(i))
                    std::this_thread::yield();
                s += i;
            }
            sum_sent.fetch_add(s, std::memory_order_relaxed);
        });
    }

    std::vector<std::thread> consumers;
    for (int t = 0; t < CONSUMERS; ++t) {
        consumers.emplace_back([&] {
            long long s = 0;
            while (recv_count.load(std::memory_order_relaxed) < TOTAL) {
                int val;
                if (q.pop(val)) {
                    s += val;
                    recv_count.fetch_add(1, std::memory_order_relaxed);
                } else {
                    std::this_thread::yield();
                }
            }
            sum_recv.fetch_add(s, std::memory_order_relaxed);
        });
    }

    for (auto& p : producers) p.join();
    for (auto& c : consumers) c.join();

    auto t1 = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    CHECK(recv_count.load() == TOTAL);
    CHECK(sum_sent.load() == sum_recv.load());
    std::cout << std::fixed << std::setprecision(1)
              << "  ops=" << TOTAL << "  time=" << ms << "ms"
              << "  throughput=" << TOTAL / ms / 1000.0 << " M ops/s\n";
}

// ──────────────────────────────────────────────────────────────────────────────
// 5. MSQueue 单线程正确性测试
// ──────────────────────────────────────────────────────────────────────────────

static void test_ms_single_thread() {
    section("MSQueue — 单线程正确性");

    MSQueue<std::string> q;
    CHECK(q.empty());

    q.push("hello");
    q.push("world");
    q.push("foo");
    CHECK(!q.empty());

    auto v = q.pop();
    CHECK(v.has_value() && *v == "hello");
    v = q.pop();
    CHECK(v.has_value() && *v == "world");
    v = q.pop();
    CHECK(v.has_value() && *v == "foo");
    CHECK(!q.pop().has_value());
    CHECK(q.empty());

    // 验证移动语义
    MSQueue<std::vector<int>> vq;
    std::vector<int> data = {1, 2, 3, 4, 5};
    vq.push(std::move(data));
    CHECK(data.empty());  // 数据已移出
    auto res = vq.pop();
    CHECK(res.has_value() && res->size() == 5);
}

// ──────────────────────────────────────────────────────────────────────────────
// 6. MSQueue 多线程正确性 + 吞吐量测试
// ──────────────────────────────────────────────────────────────────────────────

static void test_ms_multi_thread() {
    section("MSQueue — 多线程 (4P/4C)");

    constexpr int PRODUCERS  = 4;
    constexpr int CONSUMERS  = 4;
    constexpr int PER_THREAD = 500'000;
    constexpr int TOTAL      = PRODUCERS * PER_THREAD;

    MSQueue<int>           q;
    std::atomic<long long> sum_sent{0}, sum_recv{0};
    std::atomic<int>       recv_count{0};

    auto t0 = std::chrono::steady_clock::now();

    std::vector<std::thread> producers;
    for (int t = 0; t < PRODUCERS; ++t) {
        producers.emplace_back([&, t] {
            long long s = 0;
            int start = t * PER_THREAD + 1;
            for (int i = start; i < start + PER_THREAD; ++i) {
                q.push(i);
                s += i;
            }
            sum_sent.fetch_add(s, std::memory_order_relaxed);
        });
    }

    std::vector<std::thread> consumers;
    for (int t = 0; t < CONSUMERS; ++t) {
        consumers.emplace_back([&] {
            long long s = 0;
            while (recv_count.load(std::memory_order_relaxed) < TOTAL) {
                auto val = q.pop();
                if (val.has_value()) {
                    s += *val;
                    recv_count.fetch_add(1, std::memory_order_relaxed);
                } else {
                    std::this_thread::yield();
                }
            }
            sum_recv.fetch_add(s, std::memory_order_relaxed);
        });
    }

    for (auto& p : producers) p.join();
    for (auto& c : consumers) c.join();

    auto t1 = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    CHECK(recv_count.load() == TOTAL);
    CHECK(sum_sent.load() == sum_recv.load());
    std::cout << std::fixed << std::setprecision(1)
              << "  ops=" << TOTAL << "  time=" << ms << "ms"
              << "  throughput=" << TOTAL / ms / 1000.0 << " M ops/s\n";
}

// ──────────────────────────────────────────────────────────────────────────────
// 7. lock-free 检查（is_lock_free 报告）
// ──────────────────────────────────────────────────────────────────────────────

static void report_lock_free() {
    section("Lock-free 支持情况");

    {
        SPSCQueue<int, 8> q;
        // SPSC 只用 atomic<size_t>，x86-64 上总是 lock-free
        bool lf = std::atomic<std::size_t>{}.is_lock_free();
        std::cout << "  SPSCQueue (atomic<size_t>):  "
                  << (lf ? "lock-free" : "NOT lock-free") << "\n";
    }
    {
        MPMCQueue<int, 8> q;
        bool lf = std::atomic<std::size_t>{}.is_lock_free();
        std::cout << "  MPMCQueue (atomic<size_t>):  "
                  << (lf ? "lock-free" : "NOT lock-free") << "\n";
    }
    {
        // MSQueue 使用 16-byte atomic（TaggedPtr），需要 CMPXCHG16B。
        // 必须用堆分配 + alignas(16) 来保证 16 字节对齐检测准确。
        struct alignas(16) T16 {
            std::uint64_t a, b;
        };
        auto* p = new std::atomic<T16>{};
        std::cout << "  MSQueue   (atomic<16byte>):  "
                  << (p->is_lock_free() ? "lock-free ✓" : "NOT lock-free (uses mutex)")
                  << "\n";
        delete p;
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// main
// ──────────────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "======================================\n"
              << "  无锁队列测试套件\n"
              << "======================================\n";

    test_spsc_single_thread();
    test_spsc_multi_thread();

    test_mpmc_single_thread();
    test_mpmc_multi_thread();

    test_ms_single_thread();
    test_ms_multi_thread();

    report_lock_free();

    std::cout << "\n══ 汇总 ══\n"
              << "  PASS: " << g_pass << "\n"
              << "  FAIL: " << g_fail << "\n";

    return g_fail == 0 ? 0 : 1;
}
