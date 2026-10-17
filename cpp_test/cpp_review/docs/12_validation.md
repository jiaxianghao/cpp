# 验证记录

验证日期：2026-10-16。

环境：Linux，GCC 9.4.0，CMake 4.2.1，C++17，Debug 构建。

从 cpp_review 目录执行：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j2
ctest --test-dir build --output-on-failure
```

结果：10 个目标编译成功，10/10 个 CTest 检查通过。启用了 -Wall -Wextra -Wpedantic，构建未报告警告。

检查覆盖：构造析构顺序、RAII 异常清理、复制/移动选择、切片与虚调用、构造失败清理、基本/强异常保证、模板推导、erase-remove 与捕获、重构前后行为、五种设计模式、综合项目策略异常后收据不变。

边界：未执行其他编译器/平台、Release 或 Sanitizer 构建；未模拟内存分配失败或真实支付/网络故障。sizeof 和容器移动计数属于实现观察，不固定为跨平台期望。

教材练习是后续学习任务；当前交付的基线程序均已完成，可以直接运行。
