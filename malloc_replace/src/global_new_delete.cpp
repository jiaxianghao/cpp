#include <cstdlib>
#include <new>
#include <unistd.h>

namespace {
void* allocate(std::size_t size)
{
    // A successful zero-size allocation must still return a non-null pointer.
    for (;;) {
        if (void* ptr = std::malloc(size ? size : 1))
            return ptr;
        std::new_handler handler = std::get_new_handler();
        if (!handler)
            throw std::bad_alloc();
        handler();
    }
}
}

// Use write for tracing: C++ streams may themselves call operator new.
void* operator new(std::size_t size)
{
    constexpr char message[] = "operator new\n";
    (void)::write(STDOUT_FILENO, message, sizeof(message) - 1);
    return allocate(size);
}

void* operator new[](std::size_t size)
{
    constexpr char message[] = "operator new[]\n";
    (void)::write(STDOUT_FILENO, message, sizeof(message) - 1);
    return allocate(size);
}

void operator delete(void* ptr) noexcept
{
    constexpr char message[] = "operator delete\n";
    (void)::write(STDOUT_FILENO, message, sizeof(message) - 1);
    std::free(ptr);
}

void operator delete[](void* ptr) noexcept
{
    constexpr char message[] = "operator delete[]\n";
    (void)::write(STDOUT_FILENO, message, sizeof(message) - 1);
    std::free(ptr);
}

// Compilers may select sized deallocation for delete expressions.
void operator delete(void* ptr, std::size_t) noexcept
{
    ::operator delete(ptr);
}

void operator delete[](void* ptr, std::size_t) noexcept
{
    ::operator delete[](ptr);
}
