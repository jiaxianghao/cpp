#pragma once

#include <atomic>
#include <cstdint>
#include <optional>
#include <type_traits>

/**
 * Michael-Scott 无锁无界链表队列（MPMC）
 *
 * 算法来源：
 *   Maged M. Michael, Michael L. Scott,
 *   "Simple, Fast, and Practical Non-Blocking and Blocking Concurrent Queue
 *   Algorithms", PODC 1996.
 *
 * ABA 问题处理：
 *   使用带版本号的 TaggedPtr（指针 + 64 位递增计数器），需要 128-bit CAS
 *   （x86-64 的 CMPXCHG16B 指令）。编译时需 -mcx16 或 -march=native。
 *
 * 内存回收策略：
 *   出队时【不立即 delete】已退役的哨兵节点，而是保留完整的节点链表，
 *   由析构函数统一回收。这样做可完全避免 use-after-free：
 *     - 任意线程读取 head.ptr->next 时，该节点内存始终有效；
 *     - 代价是内存不能即时释放（最多累积历史入队次数）。
 *   如需即时回收，生产环境应配合危险指针（Hazard Pointers）或
 *   基于纪元的内存回收（EBR/RCU）使用。
 *
 * 析构正确性证明：
 *   initial_dummy → old_dummy_1 → old_dummy_2 → … → current_head → … → tail
 *   整条链路完整保留，析构时从 initial_dummy 顺序遍历即可释放所有节点。
 */
template <typename T>
class MSQueue {
private:
    struct Node {
        T                  data{};
        std::atomic<Node*> next{nullptr};

        Node() = default;
        explicit Node(T val) : data(std::move(val)) {}
    };

    /**
     * 带版本号的指针，用于消除 ABA 问题。
     * 要求 16 字节对齐以匹配 CMPXCHG16B 操作数要求。
     */
    struct alignas(16) TaggedPtr {
        Node*    ptr = nullptr;
        uint64_t tag = 0;

        bool operator==(const TaggedPtr& o) const noexcept {
            return ptr == o.ptr && tag == o.tag;
        }
        bool operator!=(const TaggedPtr& o) const noexcept {
            return !(*this == o);
        }
    };

    static_assert(sizeof(TaggedPtr) == 16,
                  "TaggedPtr must be exactly 16 bytes for 128-bit CAS");
    static_assert(std::is_trivially_copyable_v<TaggedPtr>,
                  "TaggedPtr must be trivially copyable for std::atomic");
    // Note: std::atomic<TaggedPtr>::is_always_lock_free is false on GCC 9 even
    // with -mcx16 because GCC calls __atomic_*_16 from libatomic at runtime.
    // libatomic uses CMPXCHG16B when the CPU supports it (is_lock_free()==true).
    // Link with -latomic to resolve the symbols.

    std::atomic<TaggedPtr> head_;
    std::atomic<TaggedPtr> tail_;
    Node* const            initial_dummy_;  // 指向队列创建时的第一个哨兵节点

public:
    MSQueue() : initial_dummy_(new Node{}) {
        TaggedPtr tp{initial_dummy_, 0};
        head_.store(tp, std::memory_order_relaxed);
        tail_.store(tp, std::memory_order_relaxed);
    }

    ~MSQueue() {
        // 先出队所有剩余元素（触发 T 的析构器）
        while (pop()) {}

        // 从 initial_dummy 出发遍历整条历史链表并释放所有节点。
        // 经过历次出队，链表为：
        //   initial_dummy → retired_1 → retired_2 → … → current_head → (nullptr)
        // （current_head 在 pop() 耗尽后其 next 为 nullptr）
        Node* curr = initial_dummy_;
        while (curr) {
            Node* nxt = curr->next.load(std::memory_order_relaxed);
            delete curr;
            curr = nxt;
        }
    }

    MSQueue(const MSQueue&) = delete;
    MSQueue& operator=(const MSQueue&) = delete;

    /**
     * 入队。lock-free，永不失败（仅受系统可用内存限制）。
     *
     * 步骤：
     *   1. 读取 tail 和 tail->next；
     *   2. 若 tail->next 为空，CAS 追加新节点（入队核心操作）；
     *   3. 尝试推进 tail 指针（即使失败，下一个入队线程也会帮助推进）。
     *   tail 可能暂时落后一个节点，这是算法允许的 "宽松一致性"。
     */
    void push(T val) {
        Node* new_node = new Node(std::move(val));

        while (true) {
            TaggedPtr tail = tail_.load(std::memory_order_acquire);
            Node*     next = tail.ptr->next.load(std::memory_order_acquire);

            // 再次确认 tail 未被其他线程推进（双重检查减少无效 CAS）
            if (tail != tail_.load(std::memory_order_acquire))
                continue;

            if (next == nullptr) {
                // tail 指向最后一个节点，尝试将新节点链接到末尾
                Node* expected = nullptr;
                if (tail.ptr->next.compare_exchange_weak(
                        expected, new_node,
                        std::memory_order_release,
                        std::memory_order_relaxed)) {
                    // 链接成功；尝试推进 tail（失败也无妨，下次入队会修正）
                    TaggedPtr new_tail{new_node, tail.tag + 1};
                    tail_.compare_exchange_weak(
                        tail, new_tail,
                        std::memory_order_release,
                        std::memory_order_relaxed);
                    return;
                }
            } else {
                // tail 落后于真实尾节点，帮助推进
                TaggedPtr new_tail{next, tail.tag + 1};
                tail_.compare_exchange_weak(
                    tail, new_tail,
                    std::memory_order_release,
                    std::memory_order_relaxed);
            }
        }
    }

    /**
     * 出队。队列为空时返回 std::nullopt，否则返回队首值。lock-free。
     *
     * 步骤：
     *   1. 读取 head 和 head->next；
     *   2. 若队列为空（head == tail 且 next 为空），返回 nullopt；
     *   3. 若 tail 落后，帮助推进；
     *   4. CAS 推进 head（将 next 节点升格为新哨兵），返回其数据。
     *
     * 注意：成功 CAS 后，旧 head.ptr 不 delete，由析构函数统一清理。
     */
    std::optional<T> pop() {
        while (true) {
            TaggedPtr head = head_.load(std::memory_order_acquire);
            TaggedPtr tail = tail_.load(std::memory_order_acquire);
            Node*     next = head.ptr->next.load(std::memory_order_acquire);

            if (head != head_.load(std::memory_order_acquire))
                continue;

            if (head.ptr == tail.ptr) {
                if (next == nullptr)
                    return std::nullopt;  // 队列确实为空
                // tail 落后，帮助推进
                TaggedPtr new_tail{next, tail.tag + 1};
                tail_.compare_exchange_weak(
                    tail, new_tail,
                    std::memory_order_release,
                    std::memory_order_relaxed);
            } else {
                // 先读取数据再做 CAS，防止读取已被 delete 的内存
                T val = next->data;
                TaggedPtr new_head{next, head.tag + 1};
                if (head_.compare_exchange_weak(
                        head, new_head,
                        std::memory_order_release,
                        std::memory_order_relaxed)) {
                    // 旧 head.ptr 退役（保留在链表中，析构时清理）
                    return val;
                }
            }
        }
    }

    bool empty() const noexcept {
        TaggedPtr head = head_.load(std::memory_order_acquire);
        TaggedPtr tail = tail_.load(std::memory_order_acquire);
        Node*     next = head.ptr->next.load(std::memory_order_acquire);
        return head.ptr == tail.ptr && next == nullptr;
    }
};
