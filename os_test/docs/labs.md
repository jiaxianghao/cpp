# 七个实验：先预测，再运行，再解释

所有命令均在仓库根目录执行。先 `make`，C 程序生成到 `build/`。每个实验记录三行：预测是什么、实际是什么、差异为什么。默认单次运行不到几秒，不用 root。

## 实验 1：fork / exec / wait 与地址空间

源码：[process.c](../labs/process.c)。约 20 分钟。

先预测：子进程把 `value` 改为 99，父进程会看到什么？子进程与父进程打印的地址相同吗？exec 后 PID 会改变吗？

```sh
./build/process
```

应观察到：子进程 `value=99`，父进程仍是 `value=10`；fork 两侧打印同一数值的虚拟地址；`exec image` 的 PID 与子进程相同；父进程拿到 `child_exit=7`。每次运行 PID、地址的具体数值可以变化。

这验证地址空间隔离和 exec 替换程序，不直接验证实际复制了多少物理页。父进程在 wait 后打印，因此此程序的日志有意保持清晰顺序。

修改练习：把 exec 分支退出码从 7 改为 9，同时更新父进程末尾的预期检查。再次编译，说明退出状态如何传回。若只改返回值，程序的检查失败是正常的。

验收：画出“父进程 → fork → 子进程 → exec → 退出 → 父进程 wait 返回”，标注 PID 和变量值。

## 实验 2：管道与 EOF

源码：[pipe.c](../labs/pipe.c)。约 20 分钟。

先预测：父进程写 11 字节，子进程每次最多读 4 字节，一次 read 能读完吗？父进程只停止写入但不关闭写端，会产生 EOF 吗？

```sh
./build/pipe
```

预期输出 `child: bytes=11 EOF reached`。管道是字节流，不能依赖一次 write 对应一次 read。所有写端关闭且缓冲数据被消费完，读端才得到 EOF。

修改练习：暂时注释父进程在写完后的 `close(ends[1])`，重新编译并执行 `timeout 2s ./build/pipe`。预期因等待 EOF 而超时，命令退出码为 124；父进程同时在 wait 子进程。恢复代码后再编译。没有 `timeout` 时可用 Ctrl+C 结束这次前台运行。

验收：在图上画出 fork 后父子各持有哪些管道端点，圈出多余写端如何阻止 EOF。

## 实验 3：FIFO / SJF / RR

源码：[schedule.py](../labs/schedule.py)。约 30 分钟。

先用纸笔计算 A=5、B=2、C=1 的首次运行和完成时刻，再运行：

```sh
python3 labs/schedule.py --policy fifo
python3 labs/schedule.py --policy sjf
python3 labs/schedule.py --policy rr --quantum 2
```

程序 J0/J1/J2 对应 A/B/C。平均响应时间分别为 4.00、1.33、2.00；平均周转时间分别为 6.67、4.00、5.67。逐任务结果见 [进程笔记](02-process.md)。

修改练习：比较 `--quantum 1` 和 `--quantum 10`；再用 `--bursts 20 1 1` 比较算法。这个模型忽略切换开销，所以不能用程序结果测量小时间片的真实成本。

验收：自行构造三个任务，先手算再运行，解释为什么某个指标改善而另一个可能变差。

## 实验 4：页面置换与 Belady 异常

源码：[paging.py](../labs/paging.py)。约 30 分钟。

先预测：增加一个页框后，缺页次数一定减少吗？FIFO 与 LRU 命中时是否改变队列顺序？

```sh
python3 labs/paging.py --policy fifo --frames 3
python3 labs/paging.py --policy fifo --frames 4
python3 labs/paging.py --policy lru --frames 3
python3 labs/paging.py --policy lru --frames 4
```

默认引用串下 FIFO 缺页数分别为 9、10，LRU 分别为 10、8。列表表示淘汰顺序，左边最先淘汰；不是固定物理页框编号。此处的 MISS 是页面不在模拟页框里，不是 TLB miss。

修改练习：运行 `python3 labs/paging.py --policy lru --frames 2 --refs 1 2 1 3 1`，再改为 FIFO。两者缺页分别为 3 和 4。解释第二次访问 1 如何影响后来淘汰谁。

验收：不看输出，列出每次访问后的页集合，区分集合变化和顺序变化。

## 实验 5：原子读写、锁与丢更新

源码：[counter.c](../labs/counter.c)。约 30 分钟。

先预测：两个线程各做 10000 次“加一”，拆分的原子读和原子写足够吗？

```sh
./build/counter split
./build/counter mutex
./build/counter atomic
```

输出中的目标总量 `expected=20000`。split 实际为 10000；mutex 与 atomic 都为 20000。split 模式用两次屏障强制时序：两个线程读同一个旧值、各写旧值加一、进入下一轮。它有逻辑竞态但没有对普通 C 对象的数据竞争。split 返回成功表示演示结果符合模型，不代表它正确实现了目标计数器。

修改练习：将 ROUNDS 改为 1000，预测三个结果。移除屏障需要两处一起移除，结果将不再固定，但原子 load/store 仍不构成原子读改写。不要把原子对象改成普通变量来追求“稳定复现”。

验收：画两列时序，标出哪一步覆盖了另一个线程的逻辑更新，并解释为什么锁必须覆盖完整读改写。

## 实验 6：条件变量与单槽队列

源码：[queue.c](../labs/queue.c)。约 30 分钟。

先写不变量：`full=0` 时槽位可写；`full=1` 时槽位可读。生产者在什么条件下等待？消费者呢？

```sh
./build/queue
```

预期 `items=10000 sum=50005000 expected=50005000 ordered=1`。消费者逐个校验 1 到 10000 的顺序，避免仅检查求和而遗漏丢失、重复抵消的情况。

修改练习：将 COUNT 改为 10，在持锁区域记录写入和消费事件，观察交替关系。不要用去掉 while 后“本机没失败”来证明 if 正确。

验收：解释 wait 为什么必须释放锁，返回时为什么又需要拿回锁。当前固定数量、单生产者单消费者模型无需退出标记；若扩展为多个消费者，则先设计数量分配或关闭协议。

## 实验 7：原子替换与持久化调用顺序

源码：[durable.py](../labs/durable.py)。约 25 分钟。

先预测：一个读者已打开旧文件，程序把同名路径替换为新文件后，旧描述符读到什么？

```sh
python3 labs/durable.py
```

预期旧描述符读出 `version=old`，重新打开路径读出 `version=new`。代码使用临时目录，正常运行结束自动清理。程序包含文件 fsync、replace、目录 fsync 的发布顺序；这个小例子不处理生产环境的权限继承、多写者协调或中途失败后的临时文件回收。

修改练习：在纸上列出每个发布步骤之后掉电时需要关注什么。仅把 fsync 注释掉并正常运行，通常仍得到同样输出，因此不能用这个观察证明持久性相同。真实掉电验证需要专门的故障注入环境，本包不执行。

验收：分别解释旧描述符行为、路径可见性、文件数据同步和目录同步。

## 检查与常见问题

```sh
make check
```

检查包含手算调度、FIFO 异常、LRU 命中更新以及 C 程序退出状态、线程计数与文件替换结果，每个子进程有超时。修改参数后需同步调整对应测试预期。

如果 `make` 失败，先看第一条编译错误；本实验依赖 Linux/POSIX 线程屏障，不保证原样支持 Windows 或 macOS。Python 程序无需安装包。PID 和地址变化是正常现象；数值结果不对则先确认源码是否改动、是否重新 make。
