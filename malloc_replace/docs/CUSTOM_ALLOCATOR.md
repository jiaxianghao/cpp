# 使用自己的内存分配器

本文中的命令均在项目根目录执行。

本项目通过 `LD_PRELOAD`，让 Linux 动态链接程序使用自定义的内存分配函数。

## 三种替换方式的作用范围

C++ 容器支持通过 Allocator 模板参数指定分配器。它与替换全局 `operator new/delete`、通过 `LD_PRELOAD` 替换 `malloc/free` 的作用范围不同：

| 方式 | 作用范围 | 是否需要修改源码 |
|---|---|---|
| 容器 Allocator | 指定容器的内部存储 | 需要 |
| 替换全局 `operator new/delete` | 使用相应全局运算符的 C++ 分配 | 需要提供替换实现并链接 |
| `LD_PRELOAD` 替换 `malloc/free` | 进程中通过这些动态符号进行的分配 | 通常不需要 |

例如，`std::vector<int, MyAllocator<int>>` 使用指定分配器管理元素存储。对于 `std::vector<std::string, MyAllocator<std::string>>`，该分配器只控制 vector 存放 string 对象的内存；字符串自身的字符缓冲区由字符串的分配器管理。

全局 `operator new/delete` 的替换不覆盖类专属的分配运算符，也不直接覆盖 `malloc/free`。`LD_PRELOAD` 是否影响 C++ 的 `new/delete`，取决于它们的实现是否调用被替换的动态符号。

## 1. 实现分配接口

如果只想控制指定 C++ 容器的分配，可以先运行 `src/container_allocator_demo.cpp`：

```bash
cmake -S . -B build
cmake --build build --target run_container
```

示例通过 `std::vector<int, MyAllocator<int>>` 指定容器分配器，打印首次预留空间、扩容和析构时的申请与释放。扩容后会检查元素内容是否保留。

`MyAllocator` 的底层暂时委托给 `std::allocator<T>`，用于展示容器调用自定义接口的过程，并没有实现独立内存池。接入自己的内存池时，成对替换 `allocate` 和 `deallocate` 中的底层调用，并保证类型对齐和分配失败时抛出异常。示例中的分配器无状态，所有实例可以互相释放内存，所以比较结果始终相等；带独立内存池的分配器需要按实际资源关系实现比较。

### 分配器的 `operator==` 和 `operator!=`

示例中定义了两个比较运算符：

```cpp
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
```

它们判断的是两个分配器是否兼容：一个实例申请的内存，能否交给另一个实例释放，而不是比较两个实例的地址是否相同。`!=` 的结果应当与 `==` 相反。

当前 `MyAllocator` 无独立状态，底层都使用 `std::allocator`，因此所有实例都兼容，`==` 始终返回 `true`，`!=` 始终返回 `false`。例如：

```cpp
MyAllocator<int> a;
MyAllocator<int> b;

int* p = a.allocate(10);
b.deallocate(p, 10);  // b 可以释放 a 申请的内存
```

容器会利用分配器是否兼容，决定能否直接接管另一容器的存储。例如 vector 移动赋值时，在分配器不随赋值传递的情况下：

- 分配器相等：可以直接接管原有缓冲区，之后用自己的分配器释放。
- 分配器不相等：需要使用自己的存储逐个移动元素；现有容量不足时，还需要重新申请内存。

如果以后每个分配器绑定独立内存池，就不能无条件返回 `true`。假设只有同一内存池的实例才能互相释放内存，比较逻辑可以是：

```cpp
// 示意：需要分配器实际持有并允许访问 pool 成员。
return a.pool == b.pool;  // operator==
return a.pool != b.pool;  // operator!=
```

如果不同池不能互相释放内存，却错误地报告分配器相等，容器可能将内存交给错误的池释放。

模板参数 `T` 和 `U` 允许比较不同元素类型的分配器。容器可能将元素分配器转换为内部节点类型的分配器，因此跨类型比较很常见。`noexcept` 表示比较操作不会抛出异常。

该容器示例不需要设置 `LD_PRELOAD`。以下步骤介绍进程级动态符号替换方式。

当前的 `src/allocator.c` 是分配器入口，已经用 `mmap/munmap` 实现了一个教学版本。可以直接修改其中的逻辑，保留以下函数：

```c
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);
```

如果你已经有自己的分配器，例如 `my_alloc` 和 `my_free`，可以在 `src/allocator.c` 中包装它们：

```c
#include <stddef.h>
#include "my_allocator.h"

void *malloc(size_t size)
{
    return my_alloc(size);
}

void free(void *ptr)
{
    if (ptr)
        my_free(ptr);
}
```

这段代码只展示两个接口的接入方式，还需要实现匹配的 `calloc` 和 `realloc`：

- `malloc`：返回满足普通类型对齐要求的内存，分配失败时返回 `NULL`。
- `free`：释放同一分配器返回的内存，允许传入 `NULL`。
- `calloc`：检查数量与单个元素大小相乘是否溢出，并将申请的内存清零。
- `realloc`：保留新旧大小中较小长度的数据；当请求非零大小且分配失败时，原内存仍然有效；传入 `NULL` 时按 `malloc` 处理。

替换实现时需要同步修改四个函数。当前示例的 `free/realloc` 依赖 `malloc` 写入的内部头部，不能直接与另一套分配器混用。

## 2. 将实现加入 CMake

如果所有逻辑都在 `src/allocator.c` 中，无需修改构建配置。

如果实现位于单独的 `src/my_allocator.c` 中，将 `CMakeLists.txt` 中的共享库定义改为：

```cmake
add_library(demo_allocator SHARED
    src/allocator.c
    src/my_allocator.c
)
```

如果用 C++ 实现，需要为项目启用 C++，并用 `extern "C"` 导出这些替换接口，避免函数名被 C++ 名字修饰。

## 3. 编译

在项目根目录执行：

```bash
cmake -S . -B build
cmake --build build
```

生成的文件包括：

- `build/demo`：测试程序。
- `build/libdemo_allocator.so`：自定义分配器动态库。

## 4. 运行示例

使用默认分配器：

```bash
./build/demo
```

使用自己的分配器：

```bash
LD_PRELOAD="$PWD/build/libdemo_allocator.so" ./build/demo
```

也可以一次完成编译和两种方式的运行：

```bash
cmake --build build --target run
```

`--target run` 指定本项目定义的 `run` 目标。它依赖测试程序和共享库，因此会先构建，再运行。

程序会打印四个内存分配函数的符号来源。默认情况下通常来自 `libc.so.6`，预加载后应来自 `libdemo_allocator.so`。程序还会检查内存申请、扩容后保留内容和零初始化。

## 5. 用于自己的程序

在项目根目录执行，将程序路径替换为你的实际路径：

```bash
LD_PRELOAD="$PWD/build/libdemo_allocator.so" /path/to/your_program
```

也可以从其他目录使用动态库的绝对路径：

```bash
LD_PRELOAD=/home/uisee/cpp/malloc_replace/build/libdemo_allocator.so /path/to/your_program
```

该环境变量作用于本次启动的进程及继承它的子进程，无需修改系统配置。静态链接程序不能通过这种方式替换分配器。

## 实现时需要注意

- 分配器内部不要再次调用被替换的 `malloc/calloc/realloc`，否则可能无限递归。可以用 `mmap` 获取底层内存。
- 分配器内部避免使用可能申请内存的 `printf` 等日志函数，以免递归进入分配器。
- 申请和释放必须使用兼容的实现，不能把自定义分配器的指针交给 libc 的 `free`。
- 多线程程序需要分配器支持并发调用；完整接入还需要根据目标程序补齐 `posix_memalign`、`aligned_alloc` 等接口。

当前实现仅用于演示替换过程：每次申请都建立映射，没有缓存或内存池，也没有完整覆盖扩展接口，不适合作为任意程序的生产分配器。

## 替换全局 `operator new/delete`

运行第三个示例：

```bash
cmake -S . -B build
cmake --build build --target run_global
```

`src/global_new_delete.cpp` 在全局作用域定义以下替换函数，并与 `src/global_new_delete_demo.cpp` 一起链接到可执行文件，无需设置 `LD_PRELOAD`：

```cpp
void* operator new(std::size_t size);
void* operator new[](std::size_t size);
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;
void operator delete(void* ptr, std::size_t size) noexcept;
void operator delete[](void* ptr, std::size_t size) noexcept;
```

后两个是带大小参数的释放函数，编译器可能为 delete 表达式选择它们。示例将它们转发给对应的不带大小参数版本。

示例展示三个场景：

1. `new Item` / `delete item`：先分配存储，再调用构造函数；释放时先调用析构函数，再释放存储。
2. `new Item[2]` / `delete[] items`：使用数组版本，构造和析构每个元素。
3. `std::vector<int>`：不指定自定义 Allocator，其默认分配器也会使用全局分配函数。

因此，`operator new` 负责申请存储，构造对象是 new 表达式的后续工作；`operator delete` 负责释放存储，析构由 delete 表达式先执行。优化器允许省略或合并某些 new 表达式的分配调用，因此日志次数不应作为优化构建中的固定保证。

底层实现使用 `std::malloc/std::free`。申请失败时按 `new_handler` 机制重试，没有处理函数则抛出 `std::bad_alloc`；零字节请求按至少一个字节申请。日志使用 Linux `write`，避免 C++ 流内部申请内存引起递归。

要接入自己的内存池，可成对替换该文件中 `allocate` 的 `std::malloc` 和释放函数中的 `std::free`，保持对齐、异常和释放语义。替换实现中不能再次使用普通 `new` 来分配内部数据，否则会递归调用自身。

该替换只链接到 `global_new_delete_demo`，另外两个示例不受影响。接入自己的 CMake 目标时，可以使用：

```cmake
target_sources(your_target PRIVATE src/global_new_delete.cpp)
```

本示例针对普通对齐类型，未替换 C++17 带 `std::align_val_t` 的过度对齐版本，也没有单独定义 `std::nothrow_t` 重载。类专属的 `operator new/delete` 可以绕过这里的全局替换；placement new 只在指定存储上构造对象，也不会由这些函数申请存储。直接调用 `malloc/free` 不会因替换全局 new/delete 而被替换。
