# 把原理重新连起来

先闭卷回答：一条 SELECT 怎么找到数据？一条 UPDATE 怎么保证别人读得合理、崩溃后还能恢复？回答不完整的地方就是本次复习重点。

## 1. 一条 SQL 的旅程

```mermaid
flowchart LR
    A[客户端连接] --> B[Server 层解析与语义检查]
    B --> C[优化器选择访问路径和连接计划]
    C --> D[执行器调用存储引擎]
    D --> E[InnoDB 索引与数据页]
    E --> F[Buffer Pool 命中或从磁盘读取]
    F --> G[返回并处理记录]
```

Server 层处理 SQL，InnoDB 管理记录、页、索引和事务。不要把解析、优化、页读取混成一步。优化器会结合统计信息估算代价，因此“存在索引”不等于“必然选择索引”。

**自问：** SQL 很慢，是扫描行数多、锁等待，还是磁盘读多？只看到耗时不能下结论。

## 2. 行、页、索引树

记录由字段值、记录头以及行格式所需的信息组成；可变长字段、NULL、长字段外部存储会影响布局。恢复概念即可，暂时不用背字节偏移。

InnoDB 以页组织存储和缓存，默认页大小通常是 16 KiB，用 `SHOW VARIABLES LIKE 'innodb_page_size'` 验证实例设置。页内有记录组织和页目录，表空间管理页，区由一组连续页组成。

聚簇索引的叶子保存完整行；二级索引的叶子保存索引列及主键值。用二级索引查出主键，再到聚簇索引取其他列，就是回表。实际 MVCC 可见性检查可能增加访问，因此覆盖索引是先从查询所需列理解的访问优化。

```text
WHERE user_id=42
    二级索引 (user_id,status,created_at,主键值)
            ├─ 只取这些列：可以覆盖
            └─ 还要 note：根据主键到聚簇索引取行
```

B+ 树内部节点帮助导航，叶子有序且相邻叶子可顺序访问，适合等值查找和范围扫描。页中可容纳多个键，树通常较矮；树高不是固定三层。主键越宽，二级索引通常也越大。

**自问：** 如果把 `SELECT *` 改成只取 `id,user_id,status,created_at`，为什么可能减少工作量？

官方补充：[聚簇与二级索引](https://dev.mysql.com/doc/refman/8.4/en/innodb-index-types.html)。

## 3. 联合索引记“排序”，不背口号

索引 `(user_id,status,created_at)` 先按用户排，同用户按状态排，同用户同状态按时间排。

| 查询条件 | 如何理解 |
| --- | --- |
| `user_id=42` | 找到用户 42 的连续区间 |
| `user_id=42 AND status=1` | 进一步缩小连续区间 |
| 再加时间范围 | 在该用户和状态内按时间定位 |
| 只有 `status=1` | 不是常规左前缀定位，优化器可能考虑其他路径 |
| `user_id=42 ORDER BY created_at` | 不同状态分别按时间排，不能直接认定整体时间有序 |

WHERE 条件书写先后不决定索引列顺序。“不满足最左前缀”也不等于“绝对不碰索引”，还可能全索引扫描或使用特定优化。范围条件之后的列仍可能用于过滤或覆盖，不能简单说“后面的索引全部失效”。见 [联合索引文档](https://dev.mysql.com/doc/refman/8.4/en/multiple-column-indexes.html)。

## 4. 执行计划是解释工具

普通 `EXPLAIN` 重点看：

| 字段 | 你要问的问题 |
| --- | --- |
| `type` | 点查、范围扫描、全索引扫描，还是全表扫描？ |
| `possible_keys` / `key` | 有哪些候选，最终选了哪个？ |
| `rows` / `filtered` | 预计扫描多少，过滤后剩多少？ |
| `Extra` | 覆盖、额外过滤、排序、临时表有哪些？ |

`Using index` 通常表示覆盖访问；`Using index condition` 表示索引条件下推，二者不同。`Using filesort` 表示需要额外排序，并不保证发生磁盘排序。`type=index` 是全索引扫描，也可能读很多记录。

`EXPLAIN ANALYZE` 中比较估计行数、实际行数、循环次数和耗时。多次循环的节点值要结合 loops 理解；父节点时间包含子节点工作，不能把整棵树的时间直接相加。执行计划和缓存状态会变，不把毫秒数当作固定答案。

连接查询还要看驱动侧产生多少行，以及被驱动侧每次付出多少成本；现代版本可能选嵌套循环或 Hash Join，不能假定所有连接都用一种算法。统计信息失真会影响估算，`ANALYZE TABLE` 用于更新统计信息，不是执行查询测速。

参考：[EXPLAIN 输出](https://dev.mysql.com/doc/refman/8.4/en/explain-output.html)、[EXPLAIN ANALYZE](https://dev.mysql.com/doc/refman/8.4/en/explain.html)。

## 5. Buffer Pool 与日志：为什么提交不等于刷完所有页

Buffer Pool 缓存数据页和索引页。修改缓存中的页会产生脏页，后台逐渐写回；缓存命中可以减少实际磁盘读取。日志也有自己的缓冲与写盘过程。

| 机制 | 要解决的问题 | 不能误认为 |
| --- | --- | --- |
| undo | 事务回滚、构造旧版本 | 一份完整数据库备份 |
| redo | 崩溃恢复中重放已记录的页修改 | 用户 SQL 历史或只有已提交事务的日志 |
| binlog | Server 层的变更记录，支持复制和基于备份的时间点恢复 | InnoDB 数据页恢复的替代品 |

WAL 的核心是先满足相应日志持久化要求，再允许相关脏页落盘。提交可以先让日志满足持久化策略，无需同步刷完这笔事务涉及的所有数据页。恢复时需要结合 redo 与 undo 处理崩溃前状态，不能理解成“有 redo 的事务都提交了”。参见 [redo 官方说明](https://dev.mysql.com/doc/refman/8.4/en/innodb-redo-log.html)。

开启 binlog 时，InnoDB 与 binlog 的提交协调要保证恢复时对提交结果的判断一致，常用 prepare → 写 binlog → commit 来回忆两阶段过程。`innodb_flush_log_at_trx_commit` 与 `sync_binlog` 会影响持久化策略；本项目观察配置，不用强杀实例来假装验证断电安全。

checkpoint 帮助限定崩溃恢复需要处理的范围；doublewrite 主要应对页写入不完整，解决的问题与 redo 不同。第一轮能区分用途即可。

## 6. MVCC：一个“读”到底读哪个版本

读已提交（RC）下，一般每次一致性读建立新快照；可重复读（RR）下，一般由事务内第一次一致性读建立快照，后续复用。普通 `BEGIN` 不等于立即建立快照；RR 下 `START TRANSACTION WITH CONSISTENT SNAPSHOT` 可主动建立。见 [一致性非锁定读](https://dev.mysql.com/doc/refman/8.4/en/innodb-consistent-read.html)。

用原书的方式记 Read View：记住创建者、创建时活跃的事务集合和事务 ID 边界。版本是否可见由这些信息判断，不只是比较开始时间。不可见时沿 undo 版本链寻找可见版本；自己的修改通常对自己可见。

普通 SELECT 的快照读取旧版本；`SELECT ... FOR UPDATE` 是锁定读，UPDATE/DELETE 也不能简单按旧快照改数据。长事务可能使旧版本迟迟不能清理。版本链与回滚指针的背景见 [InnoDB 多版本机制](https://dev.mysql.com/doc/refman/8.4/en/innodb-multi-versioning.html)。

**自问：** A 已建立 RR 快照，B 更新并提交后，A 普通 SELECT 与 FOR UPDATE 为什么能看到不同值？

## 7. 锁：从访问路径找锁范围

| 锁 | 主要含义 |
| --- | --- |
| S / X | 共享与排他，决定哪些访问相容 |
| 意向锁 | 表级协调信号，不意味着整张表禁止其他事务更新 |
| Record Lock | 锁索引记录 |
| Gap Lock | 约束间隙内插入 |
| Next-key Lock | 记录锁加记录前方的间隙锁 |

在 RR 下，锁定范围查询通常涉及 next-key 锁；通过完整唯一键查到已有记录时通常只需记录锁。RC 对普通搜索大幅减少间隙锁，但外键检查、重复键检查等仍有例外。锁的对象与扫描索引、条件、隔离级别相关。见 [InnoDB 锁](https://dev.mysql.com/doc/refman/8.4/en/innodb-locking.html) 和 [不同语句设置的锁](https://dev.mysql.com/doc/refman/8.4/en/innodb-locks-set.html)。

RR 下快照读通过版本可见性维持视图，锁定范围读通过锁限制插入；不要把这两种机制混成一句“MVCC 自动锁住了范围”。

死锁是形成循环等待；锁等待超时不一定是死锁。业务应缩短事务、尽量统一加锁顺序，并针对可重试错误处理整个事务。

## 8. 第一轮先不深挖的内容

页结构每个字段的字节数、优化器成本公式、redo 记录类型、undo 各种物理结构、源码函数调用链，先留到第二轮。检验是否想起来的标准是“能预测实验并解释”，不是能否背出所有术语。
