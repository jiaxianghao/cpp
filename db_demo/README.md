# 分层架构数据库应用 - Layered Database Application

一个基于分层架构设计的 C++ MySQL 数据库应用框架，提供清晰的职责分离和良好的可维护性。

## 📋 项目特点

✨ **分层架构设计** - Infrastructure、DAO、Business Logic、Application 四层分离  
🎯 **职责明确** - 每一层有清晰的职责和边界  
🔒 **类型安全** - 使用业务实体而非通用数据结构  
🛡️ **SQL注入防护** - 预处理语句和参数化查询  
⚡ **事务支持** - 完整的ACID事务管理  
🔄 **易于扩展** - 模块化设计，方便添加新功能  
👥 **团队友好** - 不同技能水平的开发者可以在不同层工作  

## 🏗️ 项目结构

```
db_util_project/
├── db_util/                   # 基础设施层 (Infrastructure)
│   ├── include/
│   │   ├── DatabaseManager.h # 数据库连接管理
│   │   └── TableManager.h    # 表操作封装
│   ├── src/
│   │   ├── DatabaseManager.cpp
│   │   └── TableManager.cpp
│   ├── examples/             # db_util 示例
│   ├── tests/                # db_util 测试
│   └── CMakeLists.txt
│
├── core/                      # 核心DAO层 ⭐
│   ├── include/
│   │   ├── entities/         # 业务实体
│   │   │   ├── User.h
│   │   │   ├── Order.h
│   │   │   └── Product.h
│   │   ├── dao/              # 数据访问对象
│   │   │   ├── UserDAO.h
│   │   │   ├── OrderDAO.h
│   │   │   └── ProductDAO.h
│   │   └── DatabaseService.h # 数据库服务单例
│   ├── src/
│   │   ├── entities/
│   │   ├── dao/
│   │   └── DatabaseService.cpp
│   └── CMakeLists.txt
│
├── modules/                   # 业务逻辑层
│   ├── user_module/          # 用户业务模块
│   │   ├── include/UserModule.h
│   │   ├── src/UserModule.cpp
│   │   └── CMakeLists.txt
│   └── order_module/         # 订单业务模块
│       ├── include/OrderModule.h
│       ├── src/OrderModule.cpp
│       └── CMakeLists.txt
│
├── app/                       # 应用层
│   ├── main.cpp              # 程序入口
│   └── CMakeLists.txt
│
├── docs/                      # 文档
│   ├── LAYERED_ARCHITECTURE.md
│   ├── BUILD_AND_RUN.md
│   ├── QUICK_START.md
│   └── ...
│
├── cmake/                     # CMake 配置
│   └── db_utilConfig.cmake.in
│
├── CMakeLists.txt            # 根构建配置
├── README.md
└── .gitignore
```

## 🚀 快速开始

### 1. 安装依赖

```bash
# Ubuntu/Debian
sudo apt-get install -y build-essential cmake libmysqlcppconn-dev mysql-server

# CentOS/RHEL
sudo yum install -y gcc-c++ cmake mysql-connector-c++-devel mysql-server

# macOS
brew install cmake mysql mysql-connector-c++
```

### 2. 配置数据库

```sql
CREATE DATABASE test_db;
CREATE USER 'testuser'@'localhost' IDENTIFIED BY 'testpass';
GRANT ALL PRIVILEGES ON test_db.* TO 'testuser'@'localhost';
FLUSH PRIVILEGES;
```

### 3. 编译项目

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 4. 运行示例

```bash
./app/layered_db_app
```

## 💡 使用示例

### 初始化数据库服务

```cpp
#include "DatabaseService.h"

// 获取单例实例
auto& dbService = core::DatabaseService::getInstance();

// 初始化连接
dbService.initialize("localhost", "testuser", "testpass", "test_db");

// 创建表结构
dbService.initializeTables();
```

### 使用业务模块

```cpp
#include "UserModule.h"
#include "OrderModule.h"

// 用户操作
modules::UserModule userModule;
userModule.registerUser("张三", "zhang@example.com", 25);

auto user = userModule.getUserProfile(1);
if (user.has_value())
{
    std::cout << "用户: " << user->name << std::endl;
}

// 订单操作
modules::OrderModule orderModule;
orderModule.createOrder(1, "笔记本电脑", 1, 5999.99);

auto orders = orderModule.getUserOrders(1);
for (const auto& order : orders)
{
    std::cout << "订单: " << order.productName << std::endl;
}
```

### 直接使用DAO

```cpp
// 获取DAO实例
auto& userDAO = core::DatabaseService::getInstance().getUserDAO();

// 创建用户
core::entities::User user("李四", "li@example.com", 30);
userDAO.create(user);

// 查询用户
auto foundUser = userDAO.findById(1);

// 更新用户
foundUser->age = 31;
userDAO.update(*foundUser);

// 删除用户
userDAO.deleteById(1);
```

### 事务处理

```cpp
auto& dbService = core::DatabaseService::getInstance();

if (dbService.beginTransaction())
{
    bool success = true;

    // 执行多个操作
    success &= orderModule.createOrder(...);
    success &= productModule.updateStock(...);

    if (success)
    {
        dbService.commitTransaction();
    }
    else
    {
        dbService.rollbackTransaction();
    }
}
```

## 📚 架构说明

### 四层架构

```
┌─────────────────────────────────────────┐
│       Application Layer (app/)          │  程序入口
└────────────────┬────────────────────────┘
                 ▼
┌─────────────────────────────────────────┐
│    Business Logic Layer (modules/)      │  业务逻辑
└────────────────┬────────────────────────┘
                 ▼
┌─────────────────────────────────────────┐
│       Core/DAO Layer (core/)            │  数据访问 ⭐
└────────────────┬────────────────────────┘
                 ▼
┌─────────────────────────────────────────┐
│    Infrastructure Layer (db_util/)      │  基础设施
└────────────────┬────────────────────────┘
                 ▼
            MySQL Database
```

### 各层职责

- **Infrastructure**: 封装MySQL Connector/C++，提供基本数据库操作
- **Core/DAO**: 定义实体和DAO，提供类型安全的数据访问
- **Business Logic**: 实现业务逻辑，调用DAO进行数据操作
- **Application**: 应用入口，初始化和协调各层

详细说明请查看 [LAYERED_ARCHITECTURE.md](docs/LAYERED_ARCHITECTURE.md)

## 🎯 为什么需要分层架构？

### 问题场景

❌ 如果每个模块都直接使用底层数据库工具：
- 代码耦合度高，难以维护
- 数据库变更需要修改多处代码
- 缺少业务抽象，代码可读性差
- 连接管理混乱

### 解决方案

✅ 使用分层架构：
- **职责分离** - 数据访问、业务逻辑、应用层各司其职
- **易于维护** - 修改影响范围可控
- **代码复用** - 多个模块共享同一套DAO
- **统一管理** - DatabaseService单例管理连接
- **类型安全** - 使用业务实体而非通用记录

## 📖 文档

- [📘 快速入门](docs/QUICK_START.md) - 5分钟上手指南
- [🏗️ 架构设计](docs/LAYERED_ARCHITECTURE.md) - 详细架构说明
- [🔧 构建运行](docs/BUILD_AND_RUN.md) - 编译和部署指南
- [📝 项目总结](docs/PROJECT_SUMMARY.md) - 项目概述
- [👥 团队指南](docs/TEAM_USAGE_GUIDE.md) - 团队协作指南

## 🔄 扩展指南

### 添加新实体

1. 在 `core/include/entities/` 创建实体类
2. 在 `core/include/dao/` 创建对应的DAO
3. 在 `DatabaseService` 中注册DAO
4. 在业务模块中使用

### 添加新业务模块

1. 在 `modules/` 下创建新目录
2. 实现业务逻辑，通过DAO访问数据
3. 创建 CMakeLists.txt
4. 在根CMakeLists.txt中添加

详细步骤请查看 [QUICK_START.md](docs/QUICK_START.md)

## ⚙️ 系统要求

- **操作系统**: Ubuntu 20.04+ / CentOS 7+ / macOS 10.15+ / Windows 10+
- **编译器**: GCC 9.4+ / Clang 10+ / MSVC 2019+
- **CMake**: 3.10+
- **MySQL**: 8.0+
- **MySQL Connector/C++**: 1.1.12+

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

## 📄 许可证

本项目仅供学习和开发使用。

## 🙏 致谢

- MySQL Connector/C++ 团队
- 所有贡献者

---

**开始使用**: 阅读 [快速入门指南](docs/QUICK_START.md) 🚀
