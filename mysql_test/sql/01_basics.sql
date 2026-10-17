USE mysql_review;
-- 运行前：口述 JOIN、WHERE、GROUP BY、HAVING、ORDER BY 的作用。
SELECT u.id, u.name, COUNT(*) AS paid_orders, SUM(o.amount) AS total
FROM users u JOIN orders o ON o.user_id = u.id
WHERE o.status = 1
GROUP BY u.id, u.name
HAVING COUNT(*) >= 30
ORDER BY total DESC, u.id
LIMIT 5;

-- 没有已支付订单的用户。当前种子数据结果为 0 行。
SELECT u.id, u.name
FROM users u
WHERE NOT EXISTS (
    SELECT 1 FROM orders o WHERE o.user_id = u.id AND o.status = 1
);

-- NULL 不等于任何值，包括 NULL；COUNT(列) 忽略 NULL。
SELECT NULL = NULL AS unknown_result, NULL IS NULL AS true_result;
SELECT COUNT(*) AS all_rows, COUNT(v) AS non_null_rows
FROM (SELECT 1 AS v UNION ALL SELECT NULL) AS t;

-- 修改后回滚：实验不会永久改变账户余额。
START TRANSACTION;
UPDATE accounts SET balance = balance - 100 WHERE id = 1;
UPDATE accounts SET balance = balance + 100 WHERE id = 2;
SELECT * FROM accounts WHERE id IN (1,2);
ROLLBACK;
SELECT * FROM accounts WHERE id IN (1,2);
