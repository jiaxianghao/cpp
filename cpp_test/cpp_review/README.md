# C++ 恢复性复习：经典知识、重构与设计模式

适合曾读过《Effective C++》《More Effective C++》《深度探索 C++ 对象模型》及 C++ 异常相关资料，现在希望重新建立知识体系的人。新增《重构》和设计模式。不假定你还记得条款编号；材料是原创复习笔记，不是原书逐条摘录。

默认节奏：**6 周，每周 5 个学习日 + 1 个复盘日 + 1 个休息日，每天 45～60 分钟**。用 C++17 恢复可用于日常开发的基本功，经典机制与现代替代写法一起学习。无第三方依赖。

## 从这里开始

1. 阅读 [六周计划](docs/00_plan.md)，完成第一天的闭卷诊断。
2. 构建并运行第一个实验。终端从本 README 所在的 `cpp_review` 目录开始：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j2
./build/01_lifetime
ctest --test-dir build --output-on-failure
```

需要 CMake 3.16+ 和支持 C++17 的编译器。本次验证环境见 [验证记录](docs/12_validation.md)。程序输出英文短句，解释与练习均为中文。每个程序内置检查，成功退出码为 0，检查失败抛出异常并以非零状态结束；Release 构建不会关闭检查。`ctest -V` 可以查看所有实验输出。

## 阅读与实验索引

|顺序|文档|源码 / 可执行程序|
|---|---|---|
|1|[生命周期与 const](docs/01_lifetime.md)|[01_lifetime.cpp](examples/01_lifetime.cpp) / `build/01_lifetime`|
|2|[RAII 与所有权](docs/02_ownership.md)|[02_ownership.cpp](examples/02_ownership.cpp) / `build/02_ownership`|
|3|[复制、移动与值语义](docs/03_copy_move.md)|[03_copy_move.cpp](examples/03_copy_move.cpp) / `build/03_copy_move`|
|4|[对象模型与多态](docs/04_object_model.md)|[04_object_model.cpp](examples/04_object_model.cpp) / `build/04_object_model`|
|5|[异常安全](docs/05_exceptions.md)|[05_exceptions.cpp](examples/05_exceptions.cpp) / `build/05_exceptions`|
|6|[模板与类型推导](docs/06_templates.md)|[06_templates.cpp](examples/06_templates.cpp) / `build/06_templates`|
|7|[STL 与 lambda](docs/07_stl.md)|[07_stl.cpp](examples/07_stl.cpp) / `build/07_stl`|
|8|[重构方法](docs/08_refactoring.md)|[08_refactoring.cpp](examples/08_refactoring.cpp) / `build/08_refactoring`|
|9|[设计模式与取舍](docs/09_patterns.md)|[09_patterns.cpp](examples/09_patterns.cpp) / `build/09_patterns`|
|10|[订单综合项目](docs/10_project.md)|[checkout.cpp](project/checkout.cpp) / `build/10_checkout`|

先预测结果，再读源码和运行，最后看 [答案与评分](docs/11_answers.md)。文档里的“动手练习”留给你修改；仓库内的基线程序已经可以直接运行，不需要先补 TODO。

## 学习边界

重点覆盖资源管理、对象生命周期、接口设计、异常保证、模板推导、容器、行为保持的重构与常用模式。并发、协程、模板元编程深水区、完整 STL 实现和全部 23 个 GoF 模式不作为本轮主线。设计模式文档给出完整分类地图，其余模式可在后续遇到实际需求时展开。

`sizeof`、容器重定位计数等是当前实现的观察值，不是跨平台固定答案。实验避免实际运行悬空访问、非虚析构删除派生对象等未定义行为。订单程序是练习模型，限定非负金额/数量且整数运算不溢出，不承担生产支付职责。

可选的内存与未定义行为检查（GCC/Clang）：

```bash
cmake -S . -B build-sanitize -DCMAKE_BUILD_TYPE=Debug -DREVIEW_SANITIZERS=ON
cmake --build build-sanitize -j2
ctest --test-dir build-sanitize --output-on-failure
```
