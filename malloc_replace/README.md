# 内存分配器示例

本项目提供三个可运行示例：通过 `LD_PRELOAD` 替换 Linux 进程的内存分配函数、为 C++ `vector` 指定 Allocator，以及替换全局 `operator new/delete`。

## 目录结构

```text
.
├── CMakeLists.txt
├── README.md
├── src/
│   ├── allocator.c                  # 基于 mmap/munmap 的教学分配器
│   ├── demo.c                       # LD_PRELOAD 替换验证
│   ├── container_allocator_demo.cpp # vector 自定义 Allocator
│   ├── global_new_delete.cpp        # 全局 new/delete 替换实现
│   └── global_new_delete_demo.cpp   # 对象、数组和默认 vector 示例
├── docs/
│   └── CUSTOM_ALLOCATOR.md          # 接入方法与原理说明
└── build/                           # CMake 生成的构建产物
```

## 编译和运行

需要 Linux、CMake 3.16 及以上版本，以及支持 C11 和 C++17 的编译器。在项目根目录执行：

```bash
cmake -S . -B build
cmake --build build
```

对比系统默认分配器与自定义动态库：

```bash
cmake --build build --target run
```

查看 vector 在预留空间、扩容和析构时调用自定义 Allocator：

```bash
cmake --build build --target run_container
```

查看全局 `operator new/delete` 对普通对象、数组和默认 vector 的影响：

```bash
cmake --build build --target run_global
```

也可以手动运行：

```bash
./build/demo
LD_PRELOAD="$PWD/build/libdemo_allocator.so" ./build/demo
./build/container_allocator_demo
./build/global_new_delete_demo
```

## 详细说明

[自定义分配器接入指南](docs/CUSTOM_ALLOCATOR.md) 包含三种替换方式的对比、接口实现要求、CMake 接入步骤，以及 Allocator 比较运算符的作用。

示例仅用于教学：C 分配器每次申请都建立映射，没有完整覆盖扩展接口；C++ Allocator 底层委托给 `std::allocator`；全局 new/delete 示例底层使用 `malloc/free`，展示普通对齐类型的分配与释放。
