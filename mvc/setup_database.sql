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

-- 创建索引
CREATE INDEX idx_username ON users(username);
CREATE INDEX idx_email ON users(email);
CREATE INDEX idx_created_at ON users(created_at);

-- 插入示例数据
INSERT INTO users (username, email) VALUES 
('admin', 'admin@example.com'),
('test_user', 'test@example.com');

-- 显示表结构
DESCRIBE users;

-- 显示数据
SELECT * FROM users;
