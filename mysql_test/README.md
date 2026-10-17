# MySQL 快速回忆实验室

适合：以前系统学习过 MySQL，现在想恢复理解与手感的人。

围绕《MySQL 是怎样运行的》的核心主题重新串起知识，不复刻原书、不按章节重读。先做 SQL 实验，再用现象找回原理。实现代码使用 SQL，暂时不引入应用框架。

## 怎么学最快

每次用 **5 分钟闭卷回忆 → 20 分钟实验 → 15 分钟解释现象 → 5 分钟记录**。执行 SQL 前先预测；结果不同就记下原因。只回读解释不清的部分。

| 时间 | 任务 | 完成标志 |
| --- | --- | --- |
| 第 1 天 | SQL 热身、查询链路、行与页 | 能画出一条查询到数据页的路径 |
| 第 2 天 | B+ 树、联合索引、回表、覆盖索引 | 能预测三个查询如何使用同一个索引 |
| 第 3 天 | EXPLAIN、成本、连接、分页 | 能拿执行计划解释两种 SQL 的差别 |
| 第 4 天 | 事务、undo、Read View、MVCC | 能亲自复现 RC 与 RR 的区别 |
| 第 5 天 | 行锁、间隙锁、死锁、redo/binlog | 能解释阻塞关系和三种日志的分工 |

只有一小时：初始化 → 跑 `02_indexes.sql` 中的覆盖索引和时间范围实验 → 做 MVCC 双会话实验 → 闭卷回答自测第 3、6、9、12 题。

## 开始运行

需要 Docker 和 Docker Compose。以下命令在本目录执行。使用独立容器和命名卷，不映射宿主机端口；通过 `docker compose exec` 连接。镜像首次启动需要下载，密码仅用于此学习容器。

```bash
docker compose up -d --wait
bash scripts/db.sh < sql/00_setup.sql
bash scripts/db.sh < sql/01_basics.sql
bash scripts/db.sh < sql/02_indexes.sql
```

初始化应显示 `user_count=1000`、`order_count=100000`，账户 1、2、10、20 的余额均为 1000。**再次运行初始化会重建学习库中的 users、orders、accounts 三张表。** 先结束所有实验事务再重建。

打开交互终端（并发实验需执行两次，分别标为 A、B）：

```bash
docker compose exec -e MYSQL_PWD=review_local mysql mysql -uroot --default-character-set=utf8mb4 mysql_review
```

观察锁和运行状态：

```bash
bash scripts/db.sh < sql/03_observe.sql
```

结束学习：

```bash
docker compose stop
# 下次继续，数据保留：
docker compose up -d --wait
```

也可以在自己准备的 MySQL 8.4 学习实例中执行 `mysql -u用户名 -p < sql/00_setup.sql`，之后同样导入其他 SQL；不要在业务库执行初始化。观察 Performance Schema 需要相应查询权限。

## 文档入口

- [原理回忆：把知识连起来](docs/01_mental_model.md)
- [实验手册：步骤、预测、预期结果与解释](docs/02_labs.md)
- [闭卷自测：问题与折叠答案](docs/03_recall.md)
- [复习记录模板与后续练习](docs/04_practice.md)
- [实际验证记录](docs/05_validation.md)

实验以 **MySQL 8.4 / InnoDB** 为基准。`EXPLAIN ANALYZE` 会实际执行查询，本项目只对 SELECT 使用。其输出为树形计划；普通 EXPLAIN 的行数是估算值。参见 [官方 EXPLAIN 文档](https://dev.mysql.com/doc/refman/8.4/en/explain.html)。

书中若出现查询缓存、旧式 redo 文件名称或旧锁监控表，请按版本区分：8.4 不应照抄旧版的配置和监控命令。本项目使用 `performance_schema.data_locks/data_lock_waits`，原理理解与具体版本命令分开记。

当前机器下载 Docker 镜像时连接被重置，因此尚未完成 8.4 容器实测。四份 SQL 和主要并发实验已在独立的本机 MySQL 8.0.42 临时实例中验证通过；使用已有学习实例的导入方式见上文。
