-- 扩展数据库结构 - 支持多实体
-- 创建数据库
CREATE DATABASE IF NOT EXISTS myapp CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 使用数据库
USE myapp;

-- 创建用户表
CREATE TABLE IF NOT EXISTS users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    email VARCHAR(100) NOT NULL UNIQUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

-- 创建产品表
CREATE TABLE IF NOT EXISTS products (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    price DECIMAL(10,2) NOT NULL,
    stock_quantity INT DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

-- 创建订单表
CREATE TABLE IF NOT EXISTS orders (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    status ENUM('PENDING', 'CONFIRMED', 'SHIPPED', 'DELIVERED', 'CANCELLED') DEFAULT 'PENDING',
    total_amount DECIMAL(10,2) DEFAULT 0.00,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

-- 创建订单项表
CREATE TABLE IF NOT EXISTS order_items (
    id INT AUTO_INCREMENT PRIMARY KEY,
    order_id INT NOT NULL,
    product_id INT NOT NULL,
    product_name VARCHAR(100) NOT NULL,
    unit_price DECIMAL(10,2) NOT NULL,
    quantity INT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (order_id) REFERENCES orders(id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE CASCADE
);

-- 创建索引
CREATE INDEX idx_users_username ON users(username);
CREATE INDEX idx_users_email ON users(email);
CREATE INDEX idx_users_created_at ON users(created_at);

CREATE INDEX idx_products_name ON products(name);
CREATE INDEX idx_products_price ON products(price);
CREATE INDEX idx_products_stock ON products(stock_quantity);
CREATE INDEX idx_products_created_at ON products(created_at);

CREATE INDEX idx_orders_user_id ON orders(user_id);
CREATE INDEX idx_orders_status ON orders(status);
CREATE INDEX idx_orders_created_at ON orders(created_at);
CREATE INDEX idx_orders_total_amount ON orders(total_amount);

CREATE INDEX idx_order_items_order_id ON order_items(order_id);
CREATE INDEX idx_order_items_product_id ON order_items(product_id);

-- 插入示例数据
INSERT INTO users (username, email) VALUES 
('admin', 'admin@example.com'),
('alice', 'alice@example.com'),
('bob', 'bob@example.com'),
('charlie', 'charlie@example.com');

INSERT INTO products (name, description, price, stock_quantity) VALUES 
('iPhone 15', '最新款iPhone手机', 7999.00, 50),
('MacBook Pro', '专业级笔记本电脑', 15999.00, 20),
('AirPods Pro', '无线降噪耳机', 1999.00, 100),
('iPad Air', '轻薄平板电脑', 4399.00, 30),
('Apple Watch', '智能手表', 2999.00, 80);

INSERT INTO orders (user_id, status, total_amount) VALUES 
(2, 'CONFIRMED', 9998.00),
(3, 'PENDING', 15999.00),
(4, 'SHIPPED', 2999.00);

INSERT INTO order_items (order_id, product_id, product_name, unit_price, quantity) VALUES 
(1, 1, 'iPhone 15', 7999.00, 1),
(1, 3, 'AirPods Pro', 1999.00, 1),
(2, 2, 'MacBook Pro', 15999.00, 1),
(3, 5, 'Apple Watch', 2999.00, 1);

-- 显示表结构
DESCRIBE users;
DESCRIBE products;
DESCRIBE orders;
DESCRIBE order_items;

-- 显示数据
SELECT 'Users:' as table_name;
SELECT * FROM users;

SELECT 'Products:' as table_name;
SELECT * FROM products;

SELECT 'Orders:' as table_name;
SELECT * FROM orders;

SELECT 'Order Items:' as table_name;
SELECT * FROM order_items;

-- 显示关联查询示例
SELECT 'Order Details:' as table_name;
SELECT 
    o.id as order_id,
    u.username,
    o.status,
    o.total_amount,
    o.created_at
FROM orders o
JOIN users u ON o.user_id = u.id;

SELECT 'Order Items Details:' as table_name;
SELECT 
    oi.order_id,
    oi.product_name,
    oi.unit_price,
    oi.quantity,
    (oi.unit_price * oi.quantity) as item_total
FROM order_items oi
ORDER BY oi.order_id;

