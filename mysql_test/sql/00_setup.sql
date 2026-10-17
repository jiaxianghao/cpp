-- 可重复执行。只重建 mysql_review 专用学习库内的三张实验表。
CREATE DATABASE IF NOT EXISTS mysql_review CHARACTER SET utf8mb4;
USE mysql_review;
DROP TABLE IF EXISTS orders;
DROP TABLE IF EXISTS accounts;
DROP TABLE IF EXISTS users;

CREATE TABLE users (
    id INT PRIMARY KEY,
    name VARCHAR(40) NOT NULL,
    city VARCHAR(20) NOT NULL
) ENGINE=InnoDB;

CREATE TABLE orders (
    id INT PRIMARY KEY,
    user_id INT NOT NULL,
    status TINYINT NOT NULL COMMENT '0待支付 1已支付 2已取消',
    amount DECIMAL(10,2) NOT NULL,
    created_at DATETIME NOT NULL,
    note VARCHAR(200) NOT NULL,
    KEY idx_user_status_time (user_id, status, created_at),
    KEY idx_created_at (created_at)
) ENGINE=InnoDB;

CREATE TABLE accounts (
    id INT PRIMARY KEY,
    balance DECIMAL(10,2) NOT NULL
) ENGINE=InnoDB;

-- 1000 个用户：数字表交叉连接，不依赖递归深度配置。
INSERT INTO users
WITH digits AS (
    SELECT 0 AS n UNION ALL SELECT 1 UNION ALL SELECT 2 UNION ALL SELECT 3
    UNION ALL SELECT 4 UNION ALL SELECT 5 UNION ALL SELECT 6
    UNION ALL SELECT 7 UNION ALL SELECT 8 UNION ALL SELECT 9
)
SELECT n + 1, CONCAT('user_', n + 1),
       ELT(MOD(n, 3) + 1, '北京', '上海', '深圳')
FROM (
    SELECT a.n + b.n * 10 + c.n * 100 AS n
    FROM digits a CROSS JOIN digits b CROSS JOIN digits c
) AS numbers;

-- 100000 条订单；每个用户 100 条，状态和用户编号不完全相关。
INSERT INTO orders
WITH digits AS (
    SELECT 0 AS n UNION ALL SELECT 1 UNION ALL SELECT 2 UNION ALL SELECT 3
    UNION ALL SELECT 4 UNION ALL SELECT 5 UNION ALL SELECT 6
    UNION ALL SELECT 7 UNION ALL SELECT 8 UNION ALL SELECT 9
)
SELECT n + 1,
       MOD(n, 1000) + 1,
       MOD(FLOOR(n / 1000), 3),
       10 + MOD(n, 9900) / 100,
       DATE_ADD('2025-01-01', INTERVAL n MINUTE),
       RPAD(CONCAT('order_', n + 1), 120, 'x')
FROM (
    SELECT a.n + b.n * 10 + c.n * 100 + d.n * 1000 + e.n * 10000 AS n
    FROM digits a CROSS JOIN digits b CROSS JOIN digits c
    CROSS JOIN digits d CROSS JOIN digits e
) AS numbers;
INSERT INTO accounts VALUES (1,1000.00),(2,1000.00),(10,1000.00),(20,1000.00);
ANALYZE TABLE users, orders, accounts;
SELECT VERSION() AS version, @@transaction_isolation AS isolation_level;
SELECT COUNT(*) AS user_count FROM users;
SELECT COUNT(*) AS order_count FROM orders;
