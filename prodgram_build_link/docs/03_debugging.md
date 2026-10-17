# 链接与装载排错速查

先问失败发生在哪个阶段：源码编译、最终链接、程序启动加载，还是运行中的显式插件加载。保留完整命令和第一条有意义的错误，再选工具。

| 现象 | 优先检查 | 证据与常见原因 |
| --- | --- | --- |
| undefined reference | `nm 调用方.o` 与 `nm 提供方.o` | 定义未参与链接、库顺序错误、命名不匹配 |
| multiple definition | 对参与链接的文件逐个 `nm` | 头文件定义全局对象、重复编入源码、两个强定义 |
| cannot find -lxxx | 构建命令中的 `-L`、实际库文件 | 开发库缺失、路径错误、库名不符合 libxxx.so/.a |
| error while loading shared libraries | `readelf -dW app`、本实验的 `ldd app` | NEEDED 与 RUNPATH、库实际位置不一致 |
| symbol lookup error | `nm -D 库.so`、`LD_DEBUG=libs,bindings` | 找到了不兼容库，或需要的动态符号没有导出 |
| dlopen 返回 NULL | 立即读取 `dlerror()` | 路径不存在、库本身的依赖缺失、立即重定位失败 |
| dlsym 查找失败 | `dlerror()`、`nm -D 库.so` | 名字拼错、C++ 修饰、隐藏/未导出符号 |
| 地址每次不同 | `readelf -h`、进程 maps | PIE/ASLR，不能直接当成 bug |
| 全局初始化中崩溃 | 调试器回溯、初始化相关符号 | 错误可能发生在进入 main 之前 |

## 工具各回答什么

```bash
file build/02/main.o                 # 文件类型与架构
readelf -h build/02/app              # ELF 头、类型、入口
readelf -SW build/02/app             # 节表
readelf -lW build/02/app              # 装载段和节到段的映射
readelf -sW build/02/data.o          # 符号属性、绑定、节索引
readelf -rW build/02/main.o          # 重定位项
nm -C build/08/main.o               # 符号与 C++ 名字解码
nm -D build/04/libanswer.so         # 动态符号
objdump -dr -Mintel build/02/main.o # 汇编与重定位对照
ar t build/03/libmathdemo.a         # 静态库成员
readelf -dW build/04/app            # 动态依赖和搜索路径
```

`-g` 保留调试信息，方便工具分析；`-O0` 降低实验观察难度。发布二进制可能被 strip，普通符号表可能缺失；动态符号仍可按需保留。没有在 nm 普通输出找到某函数，不足以证明相关机器代码绝对不存在。

## 一个完整诊断的写法

**现象：** 实验 08 bad.cpp 最终链接失败，错误显示找不到 c_add(int, int)。

**证据：** bad.o 引用 `_Z5c_addii`，api.o 提供 `c_add`。

**原因：** 调用方使用 C++ 语言链接，提供方由 C 编译器生成，两者符号不一致。

**修复：** 调用方包含带 extern "C" 保护的 api.h。

**验证：** 相同输入重新链接成功；确认调用方实际引用 c_add，运行行为符合预期。

不要只记录“加了一个参数就好了”。下次能够复用的是证据链。

## 先不要做的排错动作

不要一看到链接错误就修改运行时 LD_LIBRARY_PATH；不要一看到缺库就随便复制同名但 ABI 不兼容的库；不要通过允许重复定义掩盖普通代码中的多重定义。先确定阶段、符号和实际使用的文件。
