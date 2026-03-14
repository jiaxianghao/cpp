# 构建和运行指南

## 系统要求

### 操作系统
- Ubuntu 20.04+ / CentOS 7+ / Windows 10+ / macOS 10.15+

### 编译器
- GCC 9.4+ (Linux)
- Clang 10+ (macOS)
- MSVC 2019+ (Windows)

### 依赖库
- CMake 3.10+
- MySQL Server 8.0+
- MySQL Connector/C++ 1.1.12+

## 安装依赖

### Ubuntu/Debian

```bash
# 更新包列表
sudo apt-get update

# 安装构建工具
sudo apt-get install -y build-essential cmake pkg-config

# 安装 MySQL Connector/C++
sudo apt-get install -y libmysqlcppconn-dev

# 安装 MySQL Server（如果还没有）
sudo apt-get install -y mysql-server
```

### CentOS/RHEL

```bash
# 安装开发工具
sudo yum groupinstall "Development Tools"
sudo yum install cmake

# 安装 MySQL Connector/C++
sudo yum install mysql-connector-c++-devel

# 安装 MySQL Server
sudo yum install mysql-server
```

### macOS

```bash
# 使用 Homebrew 安装
brew install cmake
brew install mysql
brew install mysql-connector-c++
```

### Windows

1. 安装 Visual Studio 2019 或更高版本
2. 安装 CMake: https://cmake.org/download/
3. 安装 MySQL Server: https://dev.mysql.com/downloads/mysql/
4. 安装 MySQL Connector/C++: https://dev.mysql.com/downloads/connector/cpp/

## 配置数据库

### 1. 启动 MySQL 服务

```bash
# Linux
sudo systemctl start mysql

# macOS
brew services start mysql

# Windows
# 使用 MySQL Workbench 或服务管理器启动
```

### 2. 创建数据库和用户

```bash
# 登录 MySQL
mysql -u root -p

# 在 MySQL 命令行中执行
CREATE DATABASE test_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE USER 'testuser'@'localhost' IDENTIFIED BY 'testpass';
GRANT ALL PRIVILEGES ON test_db.* TO 'testuser'@'localhost';
FLUSH PRIVILEGES;
EXIT;
```

### 3. 验证连接

```bash
mysql -u testuser -p test_db
# 输入密码: testpass
# 如果成功登录，则配置正确
```

## 构建项目

### 1. 克隆或下载项目

```bash
cd /path/to/your/workspace
# 如果使用 git
git clone <repository_url>
cd db_util
```

### 2. 创建构建目录

```bash
mkdir build
cd build
```

### 3. 配置项目

```bash
# 基本配置
cmake ..

# 或指定构建类型
cmake -DCMAKE_BUILD_TYPE=Release ..

# 如果不需要构建示例程序
cmake -DBUILD_EXAMPLES=OFF ..
```

### 4. 编译

```bash
# 使用所有 CPU 核心编译
cmake --build . -j$(nproc)

# 或使用 make（Linux/macOS）
make -j$(nproc)

# Windows (Visual Studio)
cmake --build . --config Release
```

### 5. 安装（可选）

```bash
sudo cmake --install .
```

## 运行程序

### 1. 配置数据库连接

在运行程序之前，需要修改 `app/main.cpp` 中的数据库连接信息：

```cpp
// 修改为你的数据库配置
if (!dbService.initialize("localhost", "testuser", "testpass", "test_db", 3306))
{
    // ...
}
```

或者重新编译后再运行。

### 2. 运行主程序

```bash
# 从 build 目录运行
./app/layered_db_app
```

### 3. 运行示例程序（如果编译了）

```bash
# 简单易用示例
./easy_usage_example

# 简单数据库示例
./simple_db_example

# 简单表操作示例
./simple_table_example
```

## 预期输出

运行 `layered_db_app` 应该看到类似以下输出：

```
╔════════════════════════════════════════════════════════╗
║   分层架构数据库应用示例 - Layered Architecture Demo   ║
╚════════════════════════════════════════════════════════╝

============================================================

  1. 初始化数据库服务

============================================================

DatabaseService initialized successfully
✅ 数据库服务初始化成功

============================================================

  2. 创建数据库表结构

============================================================

All tables initialized successfully
✅ 所有表创建成功

============================================================

  3. 用户模块操作演示

============================================================

→ 注册新用户...
User registered successfully: 张三
User registered successfully: 李四
User registered successfully: 王五

→ 批量注册用户...
批量注册成功: 3 个用户

→ 查询所有活跃用户...
活跃用户数量: 6
  - ID: 1, 姓名: 张三, 邮箱: zhangsan@example.com, 年龄: 25
  ...

[... 更多输出 ...]
```

## 故障排除

### 问题1: 找不到 MySQL Connector/C++

**错误信息：**
```
CMake Error: MySQL Connector/C++ library not found
```

**解决方案：**
```bash
# Ubuntu/Debian
sudo apt-get install libmysqlcppconn-dev

# CentOS/RHEL
sudo yum install mysql-connector-c++-devel

# macOS
brew install mysql-connector-c++
```

### 问题2: 数据库连接失败

**错误信息：**
```
❌ 数据库服务初始化失败
Connection failed: ...
```

**解决方案：**
1. 检查 MySQL 服务是否运行
2. 验证用户名、密码、数据库名是否正确
3. 检查用户是否有足够的权限
4. 确认主机名和端口是否正确

### 问题3: 编译错误 - C++17 不支持

**解决方案：**
```bash
# 升级 GCC/Clang
sudo apt-get install g++-9

# 或指定编译器
CXX=g++-9 cmake ..
```

### 问题4: 链接错误

**错误信息：**
```
undefined reference to `sql::mysql::...`
```

**解决方案：**
```bash
# 确保安装了开发库
sudo apt-get install libmysqlcppconn-dev

# 清理并重新构建
rm -rf build
mkdir build
cd build
cmake ..
make
```

## 开发环境设置

### VSCode

1. 安装扩展：
   - C/C++
   - CMake Tools
   - MySQL

2. 在项目根目录创建 `.vscode/settings.json`:
```json
{
    "cmake.configureSettings": {
        "CMAKE_BUILD_TYPE": "Debug"
    },
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```

### CLion

1. 打开项目目录
2. CLion 会自动检测 CMakeLists.txt
3. 配置 Run/Debug Configuration
4. 设置工作目录为项目根目录

## 性能优化

### 编译优化

```bash
# Release 模式（优化性能）
cmake -DCMAKE_BUILD_TYPE=Release ..

# Debug 模式（便于调试）
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

### 数据库优化

1. 为常用查询字段添加索引
2. 使用连接池（可扩展功能）
3. 启用查询缓存
4. 批量操作使用事务

## 下一步

- 查看 [LAYERED_ARCHITECTURE.md](LAYERED_ARCHITECTURE.md) 了解架构设计
- 查看 [API_REFERENCE.md](API_REFERENCE.md) 了解API详情
- 查看源代码示例学习最佳实践
