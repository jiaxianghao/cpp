#include <cstdio>
#include <memory>
#include <vector>

template <class T>
struct MyAllocator {
    using value_type = T;

    MyAllocator() = default;

    template <class U>
    MyAllocator(const MyAllocator<U>&) noexcept {}

    T* allocate(std::size_t count)
    {
        // Replace this backing allocator with your own memory pool if needed.
        T* ptr = std::allocator<T>{}.allocate(count);
        std::printf("allocate: %zu elements, %zu bytes\n",
                    count, count * sizeof(T));
        return ptr;
    }

    void deallocate(T* ptr, std::size_t count) noexcept
    {
        std::printf("deallocate: %zu elements, %zu bytes\n",
                    count, count * sizeof(T));
        std::allocator<T>{}.deallocate(ptr, count);
    }
};

// All instances use the same backing allocator and can free each other's memory.
template <class T, class U>
bool operator==(const MyAllocator<T>&, const MyAllocator<U>&) noexcept
{
    return true;
}

template <class T, class U>
bool operator!=(const MyAllocator<T>&, const MyAllocator<U>&) noexcept
{
    return false;
}

int main()
{
    {
        std::vector<int, MyAllocator<int>> values;

        std::puts("reserve(3): allocate element storage");
        values.reserve(3);
        values.push_back(10);
        values.push_back(20);
        values.push_back(30);

        std::puts("reserve(capacity + 1): allocate new storage, release old storage");
        values.reserve(values.capacity() + 1);

        std::printf("values: %d %d %d\n", values[0], values[1], values[2]);
        if (values.size() != 3 || values[0] != 10 ||
            values[1] != 20 || values[2] != 30)
            return 1;

        std::puts("leave scope: vector releases storage");
    }
    return 0;
}
