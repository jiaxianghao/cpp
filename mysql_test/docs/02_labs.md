# 实验手册

先按 README 启动并初始化。所有实验只操作 `mysql_review`，会话 A、B 必须是两个独立连接。每段先预测再执行。并发步骤手动逐段执行，不能把整个文档一次性粘贴给一个终端。

## 实验 1：找回 SQL 手感（15 分钟）

运行 `bash scripts/db.sh < sql/01_basics.sql`。

观察：聚合返回 5 个用户；NOT EXISTS 返回 0 行；NULL 比较返回 NULL，IS NULL 返回 1；COUNT 两列分别为 2、1；转账事务内余额为 900、1100，回滚后均为 1000。

自己改写：列出所有用户和已支付订单数量，没有订单的也保留。先写再看：

<details>
<summary>参考 SQL 与易错点</summary>

```sql
SELECT u.id, COUNT(o.id) AS paid_count
FROM users u
LEFT JOIN orders o ON o.user_id = u.id AND o.status = 1
GROUP BY u.id;
```

如果把 `o.status=1` 放到 WHERE，会排除右侧为空的用户。用 COUNT(o.id) 而不是 COUNT(*)，否则空匹配也计为一行。当前数据每位用户都有订单；可在一个事务内添加 id=1001 的新用户，验证数量为 0 后 ROLLBACK。

</details>

## 实验 2：索引与执行计划（35 分钟）

打开 `sql/02_indexes.sql`，逐组执行。推荐交互终端粘贴 SQL，避免一次看太多结果。

| 组 | 执行前预测 | 观察重点 |
| --- | --- | --- |
| 左前缀 | 哪些条件能定位连续索引区间？ | `key`、`type`、估计行数 |
| 覆盖 | 添加 note 之后还覆盖吗？ | `Using index` 是否存在 |
| 排序 | status 未固定时为什么可能要额外排序？ | `Using filesort` |
| 日期 | 为什么范围写法更容易利用普通时间索引？ | 全表扫描与范围访问、实际行数 |
| 分页 | LIMIT 20 是否意味着只读 20 行？ | offset 版本需要跳过大量记录 |
| 连接 | 被驱动侧重复访问了几次？ | 计划树的访问方式与 loops |

预期的是访问工作的差异，**不强求某个固定计划或耗时**。数据规模、统计信息、版本、缓存都可能改变选择。当前没有 DATE(created_at) 的函数索引；不要把实验结论扩大为“所有函数条件都无法使用任何索引”。

分页实验的两条 SQL 在此数据集返回相同 id，但真实业务里不能把 offset 直接当主键值。游标需要上一页最后一个排序键，排序键不唯一时应追加唯一键；不支持无代价任意跳页。

练习：让用户 42 的全部订单直接按 created_at 排序，自己添加 `(user_id,created_at)` 索引，比较计划，然后删除自己添加的索引。思考它与现有索引的存储、写入成本取舍。

## 实验 3：RR 快照读与锁定读（15 分钟）

先确保 A、B 均没有未结束事务。在 B 中执行：

```sql
UPDATE accounts SET balance=1000 WHERE id=1;
COMMIT;
```

按表的行顺序操作，每格是一段可粘贴 SQL：

| 顺序 | 会话 A | 会话 B | 观察 |
| --- | --- | --- | --- |
| 1 | `SET SESSION TRANSACTION ISOLATION LEVEL REPEATABLE READ; START TRANSACTION; SELECT balance FROM accounts WHERE id=1;` | | A 返回 1000，建立快照 |
| 2 | | `START TRANSACTION; UPDATE accounts SET balance=900 WHERE id=1; COMMIT;` | B 提交新版本 |
| 3 | `SELECT balance FROM accounts WHERE id=1;` | | 仍为 1000 |
| 4 | `SELECT balance FROM accounts WHERE id=1 FOR UPDATE;` | | 900，锁定读读取当前记录 |
| 5 | `SELECT balance FROM accounts WHERE id=1; COMMIT;` | | 普通读仍为 1000，随后结束事务 |
| 6 | `SELECT balance FROM accounts WHERE id=1;` | | 新事务的普通读为 900 |

第 5 步 A 没有修改该行，不能把现象推广到“事务永远读不到自己刚写的值”。

接着恢复余额为 1000，把第 1 步改成 `READ COMMITTED` 重做：第 3 步应为 900。结束后执行 `SET SESSION TRANSACTION ISOLATION LEVEL REPEATABLE READ;` 恢复会话隔离级别。

加一道预测题：如果 A 只执行 START TRANSACTION，没有第 1 次 SELECT，就让 B 提交，A 随后的第一次普通 SELECT 会读到什么？答案是 900，因为此前尚未建立一致性读快照。

## 实验 4：行锁等待（10 分钟）

在两个会话先执行 `ROLLBACK;` 清理事务。

1. A：`START TRANSACTION; SELECT * FROM accounts WHERE id=1 FOR UPDATE;`
2. B：`SET SESSION innodb_lock_wait_timeout=120; START TRANSACTION; UPDATE accounts SET balance=balance+1 WHERE id=1;`
3. B 停在等待中。在第三个终端执行 `bash scripts/db.sh < sql/03_observe.sql`，观察等待方、阻塞方与 PRIMARY 上的锁。
4. A：`ROLLBACK;`，随后 B 的 UPDATE 应完成。
5. B：`ROLLBACK;`，撤销加 1。

如果超过 120 秒收到锁等待超时，B 先 ROLLBACK，然后重做；不要把等待超时当作事务必然全部回滚。

扩展：A 锁 id=1 时，B 更新 id=2 通常不受该记录锁阻塞。每次尝试后都 ROLLBACK。

## 实验 5：间隙锁阻止插入（10 分钟）

确保账号 id=15 不存在，且 A、B 结束前一个事务。初始化账号有 1、2、10、20。

1. A：`SET SESSION TRANSACTION ISOLATION LEVEL REPEATABLE READ; START TRANSACTION; SELECT * FROM accounts WHERE id>10 AND id<20 FOR UPDATE;`
2. 查询返回 0 行。B：`SET SESSION innodb_lock_wait_timeout=120; START TRANSACTION; INSERT INTO accounts VALUES (15,1000);`
3. B 等待；第三个终端查询锁。思考：返回 0 行为什么还能有锁？因为索引中存在需要保护的搜索间隙。
4. A：`ROLLBACK;`，B 插入完成。
5. B：`ROLLBACK;`，移除实验插入，方便重做。

把 A 的隔离级别改成 READ COMMITTED 重做这一组：此处 B 的插入应能完成，不需要等 A 释放锁。然后两侧 ROLLBACK，A 恢复 REPEATABLE READ。此实验结论只针对这里的普通索引范围搜索，不意味着 RC 完全没有间隙锁。

## 实验 6：制造一个死锁（10 分钟）

先在两侧执行 ROLLBACK。按照以下顺序，在 B 的 UPDATE 等待时切回 A：

| 顺序 | 会话 A | 会话 B |
| --- | --- | --- |
| 1 | `START TRANSACTION; UPDATE accounts SET balance=balance+1 WHERE id=1;` | |
| 2 | | `START TRANSACTION; UPDATE accounts SET balance=balance+1 WHERE id=2;` |
| 3 | | `UPDATE accounts SET balance=balance+1 WHERE id=1;` |
| 4 | `UPDATE accounts SET balance=balance+1 WHERE id=2;` | |

默认启用死锁检测时，一个事务收到 `ERROR 1213` 并被回滚，另一个可以继续。**不要猜固定牺牲 A 或 B**。两侧最后都执行 ROLLBACK，撤销存活事务的修改。

执行 `SHOW ENGINE INNODB STATUS\G`，找到 `LATEST DETECTED DEADLOCK`，画出 A 等待 B、B 等待 A 的关系。观察语句只反映最近一次死锁。

复盘：如果两个事务都先锁 1 再锁 2，这个例子还有循环等待吗？业务捕获死锁后应重试整个事务，而不是只补执行失败的最后一条语句。

## 实验 7：日志与缓存观察（10 分钟）

运行 `03_observe.sql`，然后对 orders 做一条范围查询，再运行观察脚本。记下读请求、实际磁盘读、脏页和 redo 写入计数。

这些是实例级累计值或瞬时值，后台任务也会影响；初始化已预热缓存，第二次查询不保证产生可见物理读。观察只能帮助认识指标，不能仅凭计数变化证明完整的崩溃恢复过程。

最后闭卷画：UPDATE → 修改页/产生 undo 与 redo → 提交持久化协调 → 后台刷脏页。说明 undo、redo、binlog 分别负责什么即可。
