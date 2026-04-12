cmake_minimum_required(VERSION 3.16)
project(unlock_queue CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 优化 + 警告
add_compile_options(
    -O2
    -Wall
    -Wextra
    -Wpedantic
    # 启用 CMPXCHG16B，让 std::atomic<TaggedPtr>（16 字节）成为真正的 lock-free
    -mcx16
)

add_executable(unlock_queue main.cpp)

target_include_directories(unlock_queue PRIVATE ${CMAKE_SOURCE_DIR})

# 链接 pthread（std::thread 在 GCC/Linux 上依赖 -pthread）
find_package(Threads REQUIRED)
target_link_libraries(unlock_queue PRIVATE Threads::Threads)

# GCC 需要 -latomic 为 16-byte std::atomic<TaggedPtr> 提供 __atomic_*_16 实现
# libatomic 在运行时检查 CPUID 并在支持 CMPXCHG16B 的 CPU 上实现 lock-free
target_link_libraries(unlock_queue PRIVATE atomic)
