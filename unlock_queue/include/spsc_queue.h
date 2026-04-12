#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <type_traits>

/**
 * SPSC（单生产者-单消费者）无锁有界队列
 *
 * 基于环形缓冲区实现，仅使用 load/store 原子操作，无需 CAS。
 * 利用 head/tail 的写者私有性完全消除竞争，是最高效的无锁队列形式。
 *
 * 约束：
 *   - 恰好一个线程调用 push()
 *   - 恰好一个线程调用 pop()
 *   - Capacity 必须为 2 的幂
 *
 * 内存序分析：
 *   push(): 以 relaxed 读取自己的 tail（只有自己修改），
 *           以 acquire 读取 head（需看见消费者的修改），
 *           写入数据后以 release 提交 tail。
 *   pop():  以 relaxed 读取自己的 head，
 *           以 acquire 读取 tail，
 *           读取数据后以 release 提交 head。
 */
template <typename T, std::size_t Capacity>
class SPSCQueue {
    static_assert(Capacity >= 2 && (Capacity & (Capacity - 1)) == 0,
                  "Capacity must be a power of 2");
    static_assert(std::is_default_constructible_v<T>);

    static constexpr std::size_t MASK = Capacity - 1;

    // 两个原子计数器各占独立缓存行，防止伪共享（false sharing）
    alignas(64) std::atomic<std::size_t> head_{0};  // 消费者写入，生产者读取
    alignas(64) std::atomic<std::size_t> tail_{0};  // 生产者写入，消费者读取
    alignas(64) std::array<T, Capacity> buf_{};

public:
    SPSCQueue() = default;
    ~SPSCQueue() = default;

    SPSCQueue(const SPSCQueue&) = delete;
    SPSCQueue& operator=(const SPSCQueue&) = delete;

    /**
     * 入队。队列满时返回 false，否则写入并返回 true。
     * 仅供生产者线程调用。
     */
    bool push(T val) {
        const std::size_t tail = tail_.load(std::memory_order_relaxed);
        // 需要 acquire 以同步消费者对 head 的修改
        if (tail - head_.load(std::memory_order_acquire) >= Capacity)
            return false;  // 队列已满

        buf_[tail & MASK] = std::move(val);
        // release：确保数据写入在 tail 更新前对其他线程可见
        tail_.store(tail + 1, std::memory_order_release);
        return true;
    }

    /**
     * 出队。队列空时返回 false，否则将值写入 val 并返回 true。
     * 仅供消费者线程调用。
     */
    bool pop(T& val) {
        const std::size_t head = head_.load(std::memory_order_relaxed);
        // 需要 acquire 以同步生产者对 tail 的修改
        if (head == tail_.load(std::memory_order_acquire))
            return false;  // 队列为空

        val = std::move(buf_[head & MASK]);
        // release：确保数据读取在 head 更新前完成
        head_.store(head + 1, std::memory_order_release);
        return true;
    }

    bool empty() const noexcept {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }

    std::size_t size() const noexcept {
        const std::size_t head = head_.load(std::memory_order_acquire);
        const std::size_t tail = tail_.load(std::memory_order_acquire);
        return tail - head;  // 利用无符号溢出自然处理绕回
    }

    static constexpr std::size_t capacity() noexcept { return Capacity; }
};
