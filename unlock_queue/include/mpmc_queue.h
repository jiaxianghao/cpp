#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>

/**
 * MPMC（多生产者-多消费者）无锁有界队列
 *
 * 基于 Dmitry Vyukov 的序列号算法（也称 "seqlock ring buffer"）。
 * 每个槽位携带一个单调递增的序列号，生产者和消费者通过对比序列号
 * 来判断槽位状态，无需带版本号的指针，天然免疫 ABA 问题。
 *
 * 序列号状态机（槽位 i，容量 N）：
 *   初始值     : sequence == i          （空闲，可入队）
 *   入队后     : sequence == enq_pos+1  （就绪，可出队）
 *   出队后     : sequence == deq_pos+N  （已回收，可复用）
 *
 * 约束：
 *   - 任意线程数均可调用 push()/pop()
 *   - Capacity 必须为 2 的幂
 *   - T 需要默认构造
 */
template <typename T, std::size_t Capacity>
class MPMCQueue {
    static_assert(Capacity >= 2 && (Capacity & (Capacity - 1)) == 0,
                  "Capacity must be a power of 2");
    static_assert(std::is_default_constructible_v<T>);

    static constexpr std::size_t MASK = Capacity - 1;

    struct Slot {
        // 与其他成员独占缓存行，避免伪共享
        alignas(64) std::atomic<std::size_t> sequence;
        T data;
    };

    alignas(64) std::array<Slot, Capacity> slots_;
    alignas(64) std::atomic<std::size_t>   enqueue_pos_{0};
    alignas(64) std::atomic<std::size_t>   dequeue_pos_{0};

public:
    MPMCQueue() {
        for (std::size_t i = 0; i < Capacity; ++i)
            slots_[i].sequence.store(i, std::memory_order_relaxed);
    }

    MPMCQueue(const MPMCQueue&) = delete;
    MPMCQueue& operator=(const MPMCQueue&) = delete;

    /**
     * 入队。队列满时返回 false（非阻塞），否则写入并返回 true。
     * 任意线程可安全调用。
     *
     * diff 的含义：
     *   == 0 : 槽位空闲，可以写入
     *   <  0 : 槽位被上一轮出队占用 → 队列已满
     *   >  0 : 其他生产者正在写入，重新采样
     */
    bool push(T val) {
        std::size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
        while (true) {
            Slot&       slot = slots_[pos & MASK];
            std::size_t seq  = slot.sequence.load(std::memory_order_acquire);
            std::ptrdiff_t diff =
                static_cast<std::ptrdiff_t>(seq) - static_cast<std::ptrdiff_t>(pos);

            if (diff == 0) {
                // 槽位空闲，CAS 抢占写入权
                if (enqueue_pos_.compare_exchange_weak(
                        pos, pos + 1, std::memory_order_relaxed)) {
                    slot.data = std::move(val);
                    // release：保证数据写入在序列号更新前可见
                    slot.sequence.store(pos + 1, std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) {
                return false;  // 队列已满
            } else {
                // 竞争失败，重新读取最新的 enqueue_pos
                pos = enqueue_pos_.load(std::memory_order_relaxed);
            }
        }
    }

    /**
     * 出队。队列空时返回 false（非阻塞），否则读出并返回 true。
     * 任意线程可安全调用。
     */
    bool pop(T& val) {
        std::size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
        while (true) {
            Slot&       slot = slots_[pos & MASK];
            std::size_t seq  = slot.sequence.load(std::memory_order_acquire);
            // 期望值：pos+1（入队完成后的序列号）
            std::ptrdiff_t diff =
                static_cast<std::ptrdiff_t>(seq) - static_cast<std::ptrdiff_t>(pos + 1);

            if (diff == 0) {
                // 槽位就绪，CAS 抢占消费权
                if (dequeue_pos_.compare_exchange_weak(
                        pos, pos + 1, std::memory_order_relaxed)) {
                    val = std::move(slot.data);
                    // 将序列号置为 pos+Capacity，该槽位可被下一轮复用
                    slot.sequence.store(pos + Capacity, std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) {
                return false;  // 队列为空
            } else {
                pos = dequeue_pos_.load(std::memory_order_relaxed);
            }
        }
    }

    bool empty() const noexcept {
        const std::size_t pos = dequeue_pos_.load(std::memory_order_acquire);
        const std::size_t seq =
            slots_[pos & MASK].sequence.load(std::memory_order_acquire);
        return static_cast<std::ptrdiff_t>(seq) -
                   static_cast<std::ptrdiff_t>(pos + 1) < 0;
    }

    static constexpr std::size_t capacity() noexcept { return Capacity; }
};
