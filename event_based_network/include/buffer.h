#pragma once

#include <cstring>
#include <vector>
#include <cassert>

// 简单的线性缓冲区，支持自动扩容和紧凑
class Buffer {
public:
    static constexpr size_t kInitSize = 4096;

    explicit Buffer(size_t init = kInitSize)
        : data_(init), read_idx_(0), write_idx_(0) {}

    // 可读字节数
    size_t readable() const { return write_idx_ - read_idx_; }
    // 可写字节数
    size_t writable() const { return data_.size() - write_idx_; }

    bool empty() const { return readable() == 0; }

    const char* read_ptr()  const { return data_.data() + read_idx_; }
    char*       write_ptr()       { return data_.data() + write_idx_; }

    // 写入完成后推进写指针（配合 write_ptr() 直接写场景）
    void advance_write(size_t n) {
        assert(n <= writable());
        write_idx_ += n;
    }

    // 消费 n 字节（读完后）
    void consume(size_t n) {
        assert(n <= readable());
        read_idx_ += n;
        if (read_idx_ == write_idx_) {
            read_idx_ = write_idx_ = 0; // 紧凑归零
        }
    }

    // 确保至少有 n 字节可写，不足时扩容或紧凑
    void ensure_writable(size_t n) {
        if (writable() >= n) return;
        // 先尝试紧凑（移动数据到头部）
        size_t rd = readable();
        if (rd + n <= data_.size()) {
            std::memmove(data_.data(), read_ptr(), rd);
            read_idx_  = 0;
            write_idx_ = rd;
        } else {
            data_.resize(write_idx_ + n);
        }
    }

    // 追加数据
    void append(const char* src, size_t len) {
        ensure_writable(len);
        std::memcpy(write_ptr(), src, len);
        advance_write(len);
    }

private:
    std::vector<char> data_;
    size_t read_idx_;
    size_t write_idx_;
};
