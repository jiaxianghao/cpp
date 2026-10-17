# 资料与阅读路线

本地笔记与实验为本复习包编写，未复制书中章节或官方作业代码。以下官方入口已在 2026-10-16 核对。

## 第一轮只用这三处

1. [OSTEP 官方免费章节](https://pages.cs.wisc.edu/~remzi/OSTEP/)：按章节标题查阅原文，当前目录使用英文版 1.10 的编号。中文书编号不同就按主题找。
2. [官方作业说明](https://pages.cs.wisc.edu/~remzi/OSTEP/Homework/homework.html)：各主题对应的模拟器与使用入口。先手算，再查看模拟器答案。
3. [官方作业源码](https://github.com/remzi-arpacidusseau/ostep-homework)：想继续练时，读相应子目录 README 后再运行，不需要现在下载整个仓库。

## 从本地实验接到原书

| 本地主题 | 官方章节标题 / 编号 | 第二轮练习 |
| --- | --- | --- |
| process、pipe | Processes 4；Process API 5；Direct Execution 6 | 官方 Process API 题目 |
| schedule | CPU Scheduling 7；Multi-level Feedback 8 | scheduler、MLFQ 模拟器 |
| 内存笔记、paging | Address Translation 15；Paging 18；TLBs 19；Page Tables 20；Swapping 21～22 | 地址翻译、多级页表、置换模拟器 |
| counter、queue | Threads 26；Thread API 27；Locks 28；Condition Variables 30；Semaphores 31；Bugs 32 | 线程交错与条件变量作业 |
| durable、持久化笔记 | Files and Directories 39；FS Implementation 40；Journaling 42 | VSFS 与 FSCK 模拟器 |

章节名称与编号可从[官方目录](https://pages.cs.wisc.edu/~remzi/OSTEP/)定位；模拟器入口见[官方作业页](https://pages.cs.wisc.edu/~remzi/OSTEP/Homework/homework.html)。

## 第一轮完成之后

若希望把知识变成系统编程能力，可以选[官方项目库](https://github.com/remzi-arpacidusseau/ostep-projects)中的 shell、并发或文件系统项目。先完成一个项目再选下一个，预计另留一到两周，实际时间取决于 C 熟练度和选题。

本地函数细节可以查 `man 2 fork`、`man 2 execve`、`man 2 pipe`、`man 3 pthread_cond_wait`、`man 2 fsync`。若系统未安装手册，先以本包注释和官方章节继续学习，无需为了准备环境中断复习。
