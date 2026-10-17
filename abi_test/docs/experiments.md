# 实验说明

## 运行

```bash
cd ~/cpp/abi_test
python3 src/run.py
```

依赖 Linux、g++、GNU ar、readelf、Python 3，无第三方 Python 包。

## 文件

- `src/v1/`：旧头文件和旧库实现。
- `src/v2/`：追加 PAUSE，CMD_NUM 从 2 变为 3；calculate 从 x+1 改为 x+2。
- `src/client.cpp`：调用库函数，并使用自己编译时的头文件校验命令。
- `src/layout.cpp`：安全地展示两个版本结构体的大小和字段偏移。
- `src/run.py`：构建、模拟换库、重编和重链接，并自动核对关键结果。

## 观察结果

| 操作 | calculate(10) | 调用方 CMD_NUM | 库返回的命令 | 调用方接受？ |
| --- | --- | --- | --- | --- |
| 旧程序 + 旧动态库 | 11 | 2 | STOP=1 | 是 |
| 旧程序只换新动态库 | 12 | 2 | PAUSE=2 | 否 |
| 调用方按新头文件重编 + 新动态库 | 12 | 3 | PAUSE=2 | 是 |
| 旧静态链接程序 | 11 | 2 | STOP=1 | 是 |
| 仅替换磁盘上的静态库 | 11 | 2 | STOP=1 | 是 |
| 用旧 client.o 重新链接新静态库 | 12 | 2 | PAUSE=2 | 否 |
| 新 client.o 链接新静态库 | 12 | 3 | PAUSE=2 | 是 |

重点对比第 2 行：同一个进程内，库已知道 PAUSE，调用方却仍按旧 CMD_NUM 拒绝它。枚举不是从动态库中读取的共享变量。

结构体示例在常见的 int 为 4 字节的平台上，旧布局大小为 8、gear 偏移为 4；新布局大小为 12、gear 偏移为 8。示例仅展示布局，不实施不兼容跨库访问，避免用未定义行为来验证结论。

## 手动查看

脚本运行完成后，active 目录中是新版库；dynamic_old 仍保留旧头文件编译的代码。

```bash
./src/build/dynamic_old
./src/build/dynamic_new
./src/build/static_old
./src/build/static_relinked
readelf -d src/build/dynamic_old
readelf -d src/build/static_old
```

动态程序使用 `$ORIGIN/active` 查找实验库。请在没有同名 libcore.so 的 LD_LIBRARY_PATH/LD_PRELOAD 干扰的环境下运行。

这里使用 extern "C" 简化函数符号，使用 `enum Cmd : int` 固定枚举底层类型；它们不能消除调用方旧常量或旧逻辑的问题。示例中的 valid 使用优化后的内联实现，直接呈现头文件逻辑进入调用方的效果。
