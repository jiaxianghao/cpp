USE mysql_review;
SHOW INDEX FROM orders;

-- 1. 联合索引左侧列：比较 key/type/rows/Extra，不死记具体行数。
EXPLAIN SELECT * FROM orders WHERE user_id = 42;
EXPLAIN SELECT * FROM orders WHERE user_id = 42 AND status = 1;
EXPLAIN SELECT * FROM orders WHERE status = 1;

-- 2. 覆盖查询与需要额外字段的查询。
EXPLAIN SELECT id, user_id, status, created_at
FROM orders WHERE user_id = 42 AND status = 1;
EXPLAIN SELECT id, user_id, status, created_at, note
FROM orders WHERE user_id = 42 AND status = 1;

-- 3. 等值前缀后按时间排序；观察是否出现 Using filesort。
EXPLAIN SELECT id, created_at FROM orders
WHERE user_id = 42 AND status = 1 ORDER BY created_at LIMIT 10;
EXPLAIN SELECT id, created_at FROM orders
WHERE user_id = 42 ORDER BY created_at LIMIT 10;

-- 4. 对列做函数运算 vs 使用原始列范围（两条语义相同）。
EXPLAIN ANALYZE SELECT * FROM orders WHERE DATE(created_at) = '2025-01-15';
EXPLAIN ANALYZE SELECT * FROM orders
WHERE created_at >= '2025-01-15' AND created_at < '2025-01-16';

-- 5. 深分页 vs 已知上一页末尾 id 的游标分页。
-- id 连续且无过滤的本数据集，两条都返回 90001..90020。
EXPLAIN ANALYZE SELECT id FROM orders ORDER BY id LIMIT 90000,20;
EXPLAIN ANALYZE SELECT id FROM orders WHERE id > 90000 ORDER BY id LIMIT 20;

-- 6. 连接：观察驱动侧、循环次数、被驱动侧的索引访问。
EXPLAIN ANALYZE
SELECT u.name, o.id, o.amount
FROM users u JOIN orders o ON o.user_id = u.id
WHERE u.id BETWEEN 40 AND 45 AND o.status = 1;
