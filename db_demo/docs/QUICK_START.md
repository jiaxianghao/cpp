# 快速入门指南

## 5分钟快速上手

本指南帮助你快速理解和使用分层架构的数据库应用。

## 第一步：理解架构

```
app/ (应用层)
  └─> modules/ (业务逻辑层)
        └─> core/ (DAO层 - 你主要使用的层)
              └─> db_util/ (基础设施层)
```

**关键概念：**
- **不要直接使用 db_util**，而是通过 core 层访问数据库
- **业务模块**专注业务逻辑，数据访问交给 DAO
- **DatabaseService** 是唯一的数据库服务入口

## 第二步：初始化数据库服务

在程序开始时初始化一次（通常在 main.cpp 中）：

```cpp
#include "DatabaseService.h"

int main()
{
    // 获取单例实例
    auto& dbService = core::DatabaseService::getInstance();

    // 初始化数据库连接
    if (!dbService.initialize("localhost", "user", "password", "database"))
    {
        std::cerr << "数据库初始化失败" << std::endl;
        return 1;
    }

    // 创建表结构（首次运行时）
    dbService.initializeTables();

    // ... 你的业务代码 ...

    return 0;
}
```

## 第三步：使用业务模块

### 示例1：用户注册

```cpp
#include "UserModule.h"

modules::UserModule userModule;

// 注册新用户 - 不需要写SQL！
if (userModule.registerUser("张三", "zhang@example.com", 25))
{
    std::cout << "注册成功" << std::endl;
}
```

### 示例2：查询用户

```cpp
// 获取用户资料
auto user = userModule.getUserProfile(1);
if (user.has_value())
{
    std::cout << "姓名: " << user->name << std::endl;
    std::cout << "邮箱: " << user->email << std::endl;
}

// 获取所有活跃用户
auto activeUsers = userModule.getActiveUsers();
for (const auto& u : activeUsers)
{
    std::cout << u.name << " - " << u.email << std::endl;
}
```

### 示例3：创建订单

```cpp
#include "OrderModule.h"

modules::OrderModule orderModule;

// 创建订单
if (orderModule.createOrder(1, "笔记本电脑", 1, 5999.99))
{
    std::cout << "订单创建成功" << std::endl;
}

// 查询用户的所有订单
auto orders = orderModule.getUserOrders(1);
for (const auto& order : orders)
{
    std::cout << "订单: " << order.productName
              << " - ¥" << order.totalPrice << std::endl;
}
```

## 第四步：如果需要直接使用DAO

有时业务模块还未提供某个功能，你可以直接使用DAO：

```cpp
// 获取 DAO
auto& userDAO = core::DatabaseService::getInstance().getUserDAO();

// 创建用户
core::entities::User user("李四", "li@example.com", 30);
if (userDAO.create(user))
{
    std::cout << "用户创建成功" << std::endl;
}

// 查询用户
auto foundUser = userDAO.findById(1);
if (foundUser.has_value())
{
    std::cout << "找到用户: " << foundUser->name << std::endl;
}

// 更新用户
foundUser->age = 31;
userDAO.update(*foundUser);

// 删除用户
userDAO.deleteById(1);
```

## 第五步：添加自己的业务模块

### 1. 创建模块目录结构

```bash
mkdir -p modules/my_module/include
mkdir -p modules/my_module/src
```

### 2. 创建头文件

```cpp
// modules/my_module/include/MyModule.h
#ifndef MODULES_MY_MODULE_H
#define MODULES_MY_MODULE_H

#include "entities/User.h"
#include <vector>

namespace modules {

class MyModule
{
public:
    // 你的业务方法
    bool doSomething(int userId);
    std::vector<core::entities::User> getSpecialUsers();
};

} // namespace modules

#endif
```

### 3. 实现业务逻辑

```cpp
// modules/my_module/src/MyModule.cpp
#include "MyModule.h"
#include "DatabaseService.h"

namespace modules {

bool MyModule::doSomething(int userId)
{
    // 获取需要的 DAO
    auto& userDAO = core::DatabaseService::getInstance().getUserDAO();

    // 实现你的业务逻辑
    auto user = userDAO.findById(userId);
    if (!user.has_value())
    {
        return false;
    }

    // ... 业务处理 ...

    return true;
}

std::vector<core::entities::User> MyModule::getSpecialUsers()
{
    auto& userDAO = core::DatabaseService::getInstance().getUserDAO();

    // 使用 DAO 提供的方法
    return userDAO.findByAge(18, 30);
}

} // namespace modules
```

### 4. 创建 CMakeLists.txt

```cmake
# modules/my_module/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(my_module)

set(CMAKE_CXX_STANDARD 17)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/core/include
    ${CMAKE_SOURCE_DIR}/include
)

add_library(my_module src/MyModule.cpp)
target_link_libraries(my_module core)
```

### 5. 在根 CMakeLists.txt 中添加

```cmake
add_subdirectory(modules/my_module)
```

### 6. 在你的应用中使用

```cpp
#include "MyModule.h"

modules::MyModule myModule;
myModule.doSomething(1);
```

## 常用模式

### 模式1：带事务的批量操作

```cpp
auto& dbService = core::DatabaseService::getInstance();
auto& userDAO = dbService.getUserDAO();

// 开始事务
if (dbService.beginTransaction())
{
    bool success = true;

    // 执行多个操作
    for (const auto& userData : batchData)
    {
        if (!userDAO.create(userData))
        {
            success = false;
            break;
        }
    }

    // 提交或回滚
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

### 模式2：跨DAO操作

```cpp
// 在业务模块中组合多个DAO
bool OrderModule::createOrderWithValidation(int userId, ...)
{
    auto& userDAO = core::DatabaseService::getInstance().getUserDAO();
    auto& orderDAO = core::DatabaseService::getInstance().getOrderDAO();
    auto& productDAO = core::DatabaseService::getInstance().getProductDAO();

    // 1. 验证用户存在
    if (!userDAO.findById(userId).has_value())
    {
        return false;
    }

    // 2. 验证产品存在和库存
    auto product = productDAO.findById(productId);
    if (!product || product->stock < quantity)
    {
        return false;
    }

    // 3. 创建订单
    core::entities::Order order(userId, product->name, quantity, price);
    if (!orderDAO.create(order))
    {
        return false;
    }

    // 4. 更新库存
    return productDAO.updateStock(productId, product->stock - quantity);
}
```

## 关键要点

### ✅ DO（推荐做法）

1. **使用业务模块** - 优先使用 UserModule、OrderModule 等
2. **通过 DatabaseService 获取 DAO** - 使用单例模式
3. **使用实体类** - User、Order 等，而不是通用的 TableRecord
4. **事务保护** - 多个操作需要原子性时使用事务
5. **业务验证** - 在业务层进行验证，而不是在 DAO 层

### ❌ DON'T（不要这样做）

1. **不要直接使用 DatabaseManager** - 应该通过 DAO 访问
2. **不要在业务代码中写 SQL** - 应该在 DAO 中封装
3. **不要跨层调用** - 遵循层次结构
4. **不要硬编码数据库配置** - 使用配置文件或环境变量
5. **不要忽略错误处理** - 检查返回值和 optional

## 下一步学习

1. 📖 阅读 [LAYERED_ARCHITECTURE.md](LAYERED_ARCHITECTURE.md) - 深入理解架构
2. 🔧 阅读 [BUILD_AND_RUN.md](BUILD_AND_RUN.md) - 编译和运行
3. 📚 查看 `app/main.cpp` - 完整的示例代码
4. 🚀 开始开发你自己的业务模块！

## 获取帮助

- 查看示例代码：`examples/` 目录
- 查看测试代码：`tests/` 目录
- 阅读头文件中的注释文档
