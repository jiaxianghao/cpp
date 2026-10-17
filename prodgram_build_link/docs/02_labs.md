# 逐步实验手册

所有命令默认在 `prodgram_build_link/` 下运行，先执行 `make all`。标注“预期失败”的命令应返回非零状态；不要把整份手册作为一个 `set -e` 脚本执行。地址、汇编偏移、系统库路径随环境变化，不要求逐字一致。

每次按四步走：**读源码 → 预测 → 看工具证据 → 用自己的话解释**。

## 实验 01：把构建流水线拆开

源码：`labs/01_pipeline/main.c`。

```bash
cat labs/01_pipeline/main.c
rg -n 'value=' build/01/main.i
cat build/01/main.s
file build/01/main.o build/01/app
./build/01/app
gcc -### labs/01_pipeline/main.c -o build/01/probe
```

观察：`.i` 中 `SCALE` 已变成 `3`，也包含头文件展开内容；`.s` 是可读汇编；`.o` 是 relocatable ELF，`app` 是可执行文件（默认工具链可能生成 PIE）。`-###` 打印驱动程序准备调用的命令，不实际完成编译。

即使 `-O0`，编译器也可能直接算出 `7 * 3`，不要以“必须看到乘法指令”为验收条件。

**修改练习：** 把 SCALE 改成 4，再 `make all`，预测输出。恢复 3 后再构建，保证后续 `make check` 的固定期望成立。

**验收：** 为什么 `gcc -c` 可以成功，而最终链接仍可能失败？

## 实验 02：符号、节与重定位

源码：`labs/02_symbols/{main,data,duplicate}.c`。

### A. 先认识文件布局

```bash
readelf -h build/02/data.o
readelf -SW build/02/data.o
readelf -sW build/02/data.o
readelf -lW build/02/app
```

构建显式使用 `-fno-common`，让未显式初始化的全局变量也直接出现在 BSS，避免旧 GCC 默认 COMMON 干扰观察。观察 `.o` 的类型 REL，`.bss` 通常为 NOBITS，`zero_area` 的符号大小为 4096 字节。`.o` 通常没有程序装载用的 program headers；可执行文件有 LOAD 等段及 section-to-segment 对应表。

**修改练习：** 把 `zero_area[1024]` 改为 `[2048]` 后重建；对比 `.bss` 大小与文件总大小。BSS 增长不应等量增加文件内容；符号和调试信息可能带来小幅变化。完成后恢复。

### B. 看见定义和引用

```bash
nm build/02/main.o
nm build/02/data.o
./build/02/app
# 预期失败：漏掉定义所在目标文件。
gcc -no-pie build/02/main.o -o build/02/missing
# 预期失败：同一个外部符号被重复定义。
gcc -no-pie build/02/main.o build/02/data.o labs/02_symbols/duplicate.c -o build/02/duplicate
```

预期：main.o 中 `shared`、`add` 是 U；data.o 中分别是 D、T，`local_bias` 通常是小写 d。完整程序输出 `shared=7 add=14`。两次错误分别指向缺少定义和 `shared` 重复定义。

**修改练习：** 给 data.c 中的 `shared` 加上文件级 static，预测为何 main.o 的引用无法由这个局部符号满足。恢复后构建。

### C. 跟踪待修补位置

```bash
readelf -rW build/02/main.o
objdump -dr -Mintel build/02/main.o
objdump -d -Mintel --disassemble=main build/02/app
nm -n build/02/app
rg -n 'shared|add|\.text' build/02/link.map
```

先忽略 `.debug_*` 重定位，集中看 `.rela.text`。找到 `add` 或 `shared` 的一项，再在带重定位的反汇编里找到相同引用；最后在链接后反汇编中看已解析目标。x86_64 下常见 `R_X86_64_PLT32` 或 `R_X86_64_PC32`，具体以输出为准。

**验收：** 写下一个“引用位置、符号名、重定位类型、最终目标”四元组。为什么 `readelf -r app` 仍可能有动态重定位？因为本程序仍有 libc 等动态依赖，不是所有引用都在静态链接阶段处理完。

## 实验 03：静态库按需抽取与顺序

```bash
ar t build/03/libmathdemo.a
nm -s build/03/libmathdemo.a
nm build/03/app | rg 'twice|unused_function'
./build/03/app
# 预期失败：库扫描早于对 twice 的引用。
gcc -Lbuild/03 -lmathdemo build/03/main.o -o build/03/wrong-order
# 正确顺序。
gcc build/03/main.o -Lbuild/03 -lmathdemo -o build/03/right-order
```

预期：归档有 twice.o、unused.o；app 中存在 twice，不存在 unused_function；输出 `twice=42`。

**修改练习：** 在 main.c 声明并调用 unused_function，重建后用 nm 再看它是否出现。恢复源码。

**验收：** 用“扫描到库时尚未解决的符号”解释顺序错误。不要归纳成“静态库不能放前面”的绝对语法规则。

## 实验 04：共享库、查找路径与符号解析

### A. 构建时与运行时查找

```bash
readelf -dW build/04/app
readelf -dW build/04/libanswer.so
ldd build/04/app
./build/04/app
(cd /tmp && /home/uisee/cpp/prodgram_build_link/build/04/app)
```

预期：app 有 `NEEDED libanswer.so`、`RUNPATH [$ORIGIN]`，库有 SONAME `libanswer.so`。两次执行都输出 `answer=42`，因为库查找使用程序目录。库名与系统路径可以从 ldd 确认。

**故障练习：** 下面用临时目录放置同一个 app，先不放库，预期加载失败；补库后恢复。使用普通实验环境，不设置额外 LD_LIBRARY_PATH。

```bash
lab_dir=$(mktemp -d /tmp/link-review.XXXXXX)
cp build/04/app "$lab_dir/app"
"$lab_dir/app"  # 预期找不到 libanswer.so
cp build/04/libanswer.so "$lab_dir/libanswer.so"
"$lab_dir/app"  # answer=42
```

目录刻意保留在 `/tmp/link-review.*`，便于检查。`$ORIGIN` 随可执行文件位置变化。

### B. PLT/GOT、预加载与立即绑定

```bash
objdump -d -Mintel --disassemble=main build/04/app
readelf -rW build/04/app
nm -D build/04/libanswer.so
LD_PRELOAD="$PWD/build/04/override.so" ./build/04/app
LD_DEBUG=libs,bindings ./build/04/app 2> build/04/loader.log
rg 'libanswer|answer' build/04/loader.log
LD_BIND_NOW=1 LD_DEBUG=bindings ./build/04/app 2> build/04/bind-now.log
```

预期：常见配置下 main 调用 `answer@plt`，动态重定位中有 answer 的 JUMP_SLOT；预加载后输出 `answer=100`。调试日志包含库搜索和绑定信息。

实验链接时显式使用 `-Wl,-z,lazy`，以便与 LD_BIND_NOW 对比；此选项仅用于教学。观察普通日志中 answer 的绑定与 `transferring control` 的先后位置；立即绑定时该绑定应更早完成。如果普通程序已经启用 NOW，首次调用时的延迟解析不再适用，可用 `readelf -dW` 检查 FLAGS/FLAGS_1。

**验收：** 程序文件没有变，返回值为什么变了？`-L` 为什么不能修复程序搬家后缺库？

## 实验 05：运行时插件加载

```bash
./build/05/app ./build/05/plugin.so
readelf -dW build/05/app
nm -D build/05/plugin.so
# 两次均预期失败，但失败阶段不同。
./build/05/app ./build/05/no-such-plugin.so
./build/05/app ./build/04/libanswer.so
```

预期：正确输出 `plugin=73`；app 的 NEEDED 中没有 plugin.so，它的路径来自命令行。缺文件在 dlopen 失败；libanswer.so 可以被打开，但不导出 plugin_value，于是 dlsym 失败。

源码先清除旧的 dlerror，再通过新的 dlerror 判断查询错误。仅凭 dlsym 返回空指针不能一般性地判定查询失败。

**修改练习：** 改 plugin.c 的返回值，只重建 plugin.so；app 无需重新链接即可读到新结果。恢复 73 后重建。

**验收：** 使用 RTLD_NOW 是否意味着以后 dlsym 任意名字都能成功？不意味着；它处理库所需重定位，不能凭空创建导出。

## 实验 06：ELF 装载与进程地址空间

```bash
readelf -h build/06/app
readelf -lW build/06/app
./build/06/app > build/06/run1.txt
./build/06/app > build/06/run2.txt
cat build/06/run1.txt
diff -u build/06/run1.txt build/06/run2.txt
```

diff 有差异时返回 1，是正常现象。程序先打印全局、常量、栈与 malloc 指针，再读取自身 `/proc/self/maps`，这样 PID、地址和映射来自同一个进程。

在每行的起止地址内找指针，记录权限及文件名：可写全局通常落在 app 的可写映射；常量在只读映射；局部变量在 `[stack]`；小块分配常在 `[heap]`，大块可能在匿名映射。不要提前把“可能”当成固定结果。

**验收：** 找一个 LOAD 段，对比 FileSiz 与 MemSiz；解释多出的空间。两次地址是否变化受 ASLR 环境控制，不应把必须变化作为测试条件。

**延伸思考：** 为什么 maps 只能帮助你认识虚拟映射，而不能直接证明每页已经占用独立物理内存？

## 实验 07：main 前后与退出路径

```bash
./build/07/app
./build/07/app exit
./build/07/app quick
readelf -h build/07/app
nm -n build/07/app | rg '_start| main$'
readelf -SW build/07/app | rg 'init_array|fini_array'
```

正常 return 的顺序：

```text
construct global
enter main
construct local
return from main
destroy local
atexit callback
destroy global
```

exit 路径没有 `return from main` 和 `destroy local`，但有退出回调及全局析构。quick 路径调用 `_Exit`，只留下前三行。

可选 GDB：

```bash
gdb -q build/07/app
```

在 GDB 中依次执行 `starti`、`info files`、`break main`、`continue`、`bt`、`quit`。动态程序的 starti 可能先停在动态链接器；ELF 入口也不等于 main 地址。若环境禁止 ptrace，跳过 GDB 即可，不影响主实验。

**验收：** 能否用 exit 替代层层 return 并期待所有栈对象自动析构？为什么？

## 实验 08：C/C++ 名字修饰与 ABI

```bash
nm build/08/api.o
nm build/08/main.o
nm -C build/08/main.o
./build/08/app
# 预期失败：C++ 调用方未声明 C 语言链接。
g++ labs/08_abi/bad.cpp build/08/api.o -o build/08/bad
# 留下错误调用方的目标文件，直接检查其引用。
g++ -c labs/08_abi/bad.cpp -o build/08/bad.o
nm build/08/bad.o
```

预期：api.o 定义 `c_add`；正确 main.o 引用 `c_add`，而 cpp_add 有修饰后的名称。bad.o 引用的通常是 `_Z5c_addii`，这与 `c_add` 不是同一个符号。`nm -C` 会把修饰名解码成更易读的形式；排错时也要看原始名称。

**修复练习：** 把 bad.cpp 的手写声明换成 `#include "api.h"` 后重新链接。这个程序成功后返回 5（函数结果），不是返回 0；不要把它的退出码 5 当成链接失败。完成后恢复故意失败的源码，以便 make check 继续验证问题。

**验收：** extern "C" 能否解决结构体布局不同、内存由不同分配器释放的问题？不能，符号命名只是 ABI 的一部分。
