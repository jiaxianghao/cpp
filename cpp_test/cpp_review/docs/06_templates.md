# 模板、类型推导与转发

## 核心回顾

模板先理解“替换后生成什么接口”，不要从复杂元编程入手。`auto value = reference` 通常推导为值并丢弃顶层 const 与引用；`auto&` 保留引用语义。`decltype(x)` 对未加括号的名字有特殊规则，而 `decltype((x))` 根据表达式类别推导，左值通常得到引用类型。

在需要推导 T 的函数模板中，`T&&` 可作为转发引用；传入左值时 T 可推导为左值引用。引用折叠规则是：只要涉及 &，结果就是 &；只有 && 与 && 组合才得到 &&。`const T&&` 不是这种转发引用；固定类型的 `Widget&&` 也不是。

有名字的参数表达式是左值，即使参数类型是右值引用。`std::forward<T>(value)` 按 T 还原调用者传入的值类别；`std::move(value)` 无条件转成适合移动的类别。把所有参数都 move 掉可能意外消耗调用者传入的左值。

泛型代码同样应说明要求。twice 使用 `value + value`，所以类型必须支持该操作。C++17 可以用编译诊断、type_traits 或有限的 SFINAE 表达要求；本轮不为了演示技巧提前搭建复杂模板框架。

## 先预测

1. auto copy 和 auto& alias 的类型分别是什么？
2. 为什么 lose_category(3) 报告 lvalue？
3. 把 forward 改成 move 后，传入 n 会怎样分类？

## 实验与解读

运行 `./build/06_templates`。static_assert 验证编译期类型，CHECK 验证重载选择。结果应为 forward 保留左值/右值，而直接使用具名参数选择左值重载。

## 动手练习

增加 const int& 对应的 category 重载，预测 forward_category 接收 const 左值时的输出。用自己的话解释为何这不需要新增模板分支。

## 过关标准

能不看笔记解释预测题，并完成练习中的修改。答案见 [答案与评分](11_answers.md)，不要用背输出代替解释。
