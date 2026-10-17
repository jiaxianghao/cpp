# 数据结构与算法：24 周恢复计划

适合：系统学过、现在遗忘较多，希望重新建立知识体系并恢复编程能力的人。

节奏：**每天 30 分钟，每周 5 天，24 周约 60 小时**。周末休息或补漏，不要求刷题数量。示例使用 **C++17**，不依赖第三方库，不需要联网。24 周是起始安排；验收没通过就延长当前模块。

## 从这里开始

1. 阅读 [复习计划](docs/PLAN.md)，了解两周一个模块的具体安排。
2. 用第一天的 30 分钟完成 [起点自测](docs/BASELINE.md)，不会的直接标记，不必先查资料。
3. 打开 [模块 01：复杂度与基本方法](docs/modules/01_foundations.md)，按计划学习。
4. 在 [进度表](docs/PROGRESS.md) 记录当天结果，用 [练习册](exercises/WORKBOOK.md) 做独立练习。

## 编译和运行

需要 CMake ≥ 3.16 和支持 C++17 的编译器。Linux 下在本目录执行：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j2
ctest --test-dir build --output-on-failure
./build/01_foundations
```

每个示例都是独立程序，运行时会先检查结果，再输出演示。测试失败会以非零状态退出。**运行通过只是参考程序正确性的部分证据，不代表你已经掌握。** 不用 CMake 时也可以单独编译：

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -I tests examples/01_foundations.cpp -o /tmp/review_foundations
/tmp/review_foundations
```

练习模板位于 `exercises/`，默认不编译，避免未完成的 TODO 混入参考程序测试。完成某题后：

```bash
cmake --build build --target practice_01
./build/practice_01
```

初始模板抛出 `TODO` 异常，退出非零属于预期行为；请实现函数后再运行。练习目标编号是 `practice_01` 至 `practice_12`。

## 资料导航

| 模块 / 周次 | 讲义 | 可运行参考程序 |
|---|---|---|
| 01 / 1–2 | [复杂度、递归、前缀和](docs/modules/01_foundations.md) | [01_foundations.cpp](examples/01_foundations.cpp) |
| 02 / 3–4 | [数组、链表、栈、队列](docs/modules/02_linear.md) | [02_linear.cpp](examples/02_linear.cpp) |
| 03 / 5–6 | [哈希、双指针、滑动窗口](docs/modules/03_hash_window.md) | [03_hash_window.cpp](examples/03_hash_window.cpp) |
| 04 / 7–8 | [排序、二分、分治](docs/modules/04_sort_search.md) | [04_sort_search.cpp](examples/04_sort_search.cpp) |
| 05 / 9–10 | [树与二叉搜索树](docs/modules/05_tree.md) | [05_tree.cpp](examples/05_tree.cpp) |
| 06 / 11–12 | [堆与并查集](docs/modules/06_heap_dsu.md) | [06_heap_dsu.cpp](examples/06_heap_dsu.cpp) |
| 07 / 13–14 | [图遍历与拓扑排序](docs/modules/07_graph.md) | [07_graph.cpp](examples/07_graph.cpp) |
| 08 / 15–16 | [最短路与最小生成树](docs/modules/08_shortest_path.md) | [08_shortest_path.cpp](examples/08_shortest_path.cpp) |
| 09 / 17–18 | [回溯与剪枝](docs/modules/09_backtracking.md) | [09_backtracking.cpp](examples/09_backtracking.cpp) |
| 10 / 19–20 | [贪心与正确性证明](docs/modules/10_greedy.md) | [10_greedy.cpp](examples/10_greedy.cpp) |
| 11 / 21–22 | [动态规划](docs/modules/11_dp.md) | [11_dp.cpp](examples/11_dp.cpp) |
| 12 / 23–24 | [字符串、区间查询与综合复习](docs/modules/12_string_range.md) | [12_string_range.cpp](examples/12_string_range.cpp) |

其他资料：

- [学习方法与 C++ 热身](docs/METHOD.md)：每天怎么做、怎么调试、STL 和所有权。
- [练习册](exercises/WORKBOOK.md)：每模块 1 个带测试模板的必做题、2 个拓展题。
- [分层提示与参考思路](docs/SOLUTIONS.md)：卡住后再看，不作为第一遍阅读材料。
- [复杂度和算法选型速查](docs/CHEATSHEET.md)。
- [进阶知识地图](docs/ADVANCED.md)：平衡树、Trie、线段树、数论等后续方向。
- [综合项目与验收](docs/CAPSTONE.md)：任务依赖分析器，分成 30 分钟的小任务。
- [进度与错题模板](docs/PROGRESS.md)。

参考程序为教学规模实现，输入约定写在代码/讲义中；重点是算法与不变量，不是生产环境的通用容器。讲义区分“必须能手写”“能解释和选型”“后续选学”，不要求在 60 小时内掌握全部高级算法。
