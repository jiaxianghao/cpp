# 闭卷自测

每题先口述 30～60 秒，再展开答案。0 分：不会；1 分：记得词但解释不清；2 分：能用实验解释。不要把总分当证书，把 0 分和 1 分的题作为下次入口。

## 1. Server 层和 InnoDB 分别做什么？

<details><summary>答案</summary>

Server 层解析 SQL、优化执行计划、执行语句；InnoDB 负责记录和索引存储、页缓存、事务与锁。查询计划决定怎么访问，存储引擎完成相应访问。

</details>

## 2. 为什么以页组织数据？B+ 树为什么适合范围查询？

<details><summary>答案</summary>

以页为单位组织缓存和 I/O，可在一次读取中取得多条记录。B+ 树内部节点容纳多个导航键，叶子记录有序且可沿相邻叶子遍历。树高取决于键宽、数据量和填充情况。

</details>

## 3. 二级索引找到记录后，为什么还可能回表？

<details><summary>答案</summary>

二级索引通常不保存整行，只包含索引列和主键值。查询需要 note 等其他列时，要到聚簇索引取行。查询字段被索引覆盖时可以减少这些访问。对应索引实验第 2 组。

</details>

## 4. 索引 (a,b,c) 能直接满足 WHERE a=1 ORDER BY c 吗？

<details><summary>答案</summary>

不能仅凭这个索引断定。a 固定之后，仍先按 b 排，再在同一个 b 内按 c 排。不同 b 之间的 c 不保证有序。对应实验中的 user_id、status、created_at。

</details>

## 5. 有索引为什么还全表扫描？Using filesort 一定写磁盘吗？

<details><summary>答案</summary>

选择率低、回表成本高、条件无法有效缩小索引范围或统计估计等因素，都可能使全表扫描更便宜。Using filesort 表示额外排序，可能只在内存完成。需要结合实际计划，不只检查索引是否存在。

</details>

## 6. EXPLAIN 和 EXPLAIN ANALYZE 有何不同？

<details><summary>答案</summary>

前者展示计划与估算，后者实际执行查询并输出运行信息。估算 rows 不是实际计数。ANALYZE 的耗时受缓存、并发等影响，观察 rows 和 loops 往往比死记毫秒更有用。

</details>

## 7. LIMIT 90000,20 为什么可能慢？

<details><summary>答案</summary>

执行器通常要先处理并跳过前面大量记录，返回 20 行不意味着只访问 20 行。已知上一页末尾排序键时可用范围条件继续扫描，但不能用它无代价随机跳转任意页。

</details>

## 8. COMMIT 为什么不用等待所有脏页刷完？

<details><summary>答案</summary>

通过 WAL 与日志持久化策略，崩溃后可恢复尚未落盘的修改，脏页可以稍后写回。持久性保证还取决于配置与存储系统，不能只凭客户端收到成功就忽略这些条件。

</details>

## 9. undo、redo、binlog 各做什么？

<details><summary>答案</summary>

undo 支持回滚和旧版本构造；redo 用于 InnoDB 崩溃恢复；binlog 记录 Server 层变更，用于复制及结合备份进行时间点恢复。三者不是同一个日志的三种叫法。

</details>

## 10. RR 的快照在 BEGIN 时必然建立吗？RC 呢？

<details><summary>答案</summary>

普通 BEGIN 不保证立即建立快照。RR 通常在第一次一致性读建立并复用，显式 WITH CONSISTENT SNAPSHOT 是另一种情况。RC 每次一致性读通常获得新快照。对应实验 3。

</details>

## 11. MVCC 如何找到旧版本？长事务有什么影响？

<details><summary>答案</summary>

按事务信息和 Read View 判断版本可见性，必要时沿 undo 版本链找可见版本。长事务的旧快照可能阻碍历史版本清理；如果还持锁，会延长锁占用时间。

</details>

## 12. 同一 RR 事务中，普通 SELECT 和 FOR UPDATE 为什么可能不同？

<details><summary>答案</summary>

普通一致性读使用已有快照，锁定读访问当前记录并加锁，不受同一旧快照限制。实验 3 中分别读出 1000、900。后续自己再修改该行时还需考虑“读己之写”。

</details>

## 13. SELECT 返回 0 行，为什么还能阻塞插入？

<details><summary>答案</summary>

RR 下的锁定范围查询可以保护搜索间隙，防止其他事务往该范围插入。有没有结果记录，与有没有需要保护的索引间隙，是两个问题。对应实验 5。

</details>

## 14. 如何区分死锁和普通等待？

<details><summary>答案</summary>

普通等待可能仅是单向依赖，循环等待才构成死锁。用 data_locks、data_lock_waits 和 InnoDB 状态观察。死锁检测通常回滚一个事务；锁等待超时默认不意味着整个事务都已回滚。对应实验 4、6。

</details>

## 15. 发生死锁，为什么不能只重试最后一条 SQL？

<details><summary>答案</summary>

被选中牺牲的事务已被整体回滚，前面的修改也丢失。需要从业务事务边界重新执行，并考虑重试次数和业务幂等性。本轮只需理解事务边界，不必先写复杂重试框架。

</details>

## 16. 用 ACID 解释一次转账

<details><summary>答案</summary>

原子性：扣款与入账一起成功或撤销；一致性：转账前后满足余额、总额等业务约束，需要应用逻辑与数据库共同保证；隔离性：并发操作遵循隔离级别和锁规则；持久性：成功提交的结果按相应持久化保证保存。仅有事务语法不代表业务条件自动正确。

</details>
