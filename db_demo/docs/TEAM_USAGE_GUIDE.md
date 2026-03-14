# 团队数据库操作指南

## 概述

为了让团队中不熟悉SQL的成员也能方便地操作数据库，我们提供了`TableManager`类，它提供了简单易用的API，无需编写SQL语句就能完成常见的数据库操作。

## 快速开始

### 1. 基本设置

```cpp
#include "TableManager.h"

// 创建数据库连接
DatabaseManager dbManager;
dbManager.connect("localhost", "username", "password", "database", 3306);

// 创建表管理器
TableManager tableManager(dbManager);
```

### 2. 创建表

```cpp
// 定义表结构
std::map<std::string, std::string> userFields = {
    {"id", "INT AUTO_INCREMENT PRIMARY KEY"},
    {"name", "VARCHAR(100) NOT NULL"},
    {"email", "VARCHAR(100) UNIQUE NOT NULL"},
    {"age", "INT"},
    {"is_active", "BOOLEAN DEFAULT TRUE"}
};

// 创建表
tableManager.createTable("users", userFields);
```

## 数据操作

### 插入数据

```cpp
// 创建记录
TableRecord user;
user.set("name", "张三");
user.set("email", "zhangsan@example.com");
user.set("age", 25);
user.set("is_active", true);

// 插入单条记录
tableManager.insert("users", user);

// 批量插入
std::vector<TableRecord> users;
// ... 添加多个用户记录
tableManager.insertBatch("users", users);
```

### 查询数据

```cpp
// 查询所有记录
auto allUsers = tableManager.select("users");

// 查询特定字段
auto nameEmails = tableManager.select("users", {"name", "email"});

// 条件查询
auto activeUsers = tableManager.selectWhere("users", "is_active = 1");

// 查询单个记录
auto user = tableManager.selectById("users", "id", 1);

// 查询单个记录（条件）
auto user = tableManager.selectOne("users", "email = 'zhangsan@example.com'");
```

### 更新数据

```cpp
// 更新特定记录
TableRecord updateData;
updateData.set("age", 26);
updateData.set("is_active", false);

// 通过ID更新
tableManager.updateById("users", updateData, "id", 1);

// 条件更新
tableManager.update("users", updateData, "email = 'zhangsan@example.com'");
```

### 删除数据

```cpp
// 删除特定记录
tableManager.deleteById("users", "id", 1);

// 条件删除
tableManager.deleteWhere("users", "age < 18");
```

### 统计查询

```cpp
// 统计总记录数
int totalUsers = tableManager.count("users");

// 条件统计
int activeUsers = tableManager.countWhere("users", "is_active = 1");
```

## 复杂查询

### 使用查询构建器

```cpp
// 构建复杂查询
auto query = tableManager.query()
    .select({"name", "email", "age"})
    .from("users")
    .where("age > 25")
    .andWhere("is_active = 1")
    .orderBy("age", false)  // 降序
    .limit(10);

// 执行查询
std::string sql = query.build();
auto result = dbManager.executeSelect(sql);
```

## 事务处理

```cpp
// 开始事务
if (tableManager.beginTransaction())
{
    // 执行多个操作
    bool success = tableManager.insert("users", user1) &&
                  tableManager.insert("users", user2) &&
                  tableManager.update("users", updateData, "id = 1");
    
    if (success)
    {
        tableManager.commitTransaction();
    }
    else
    {
        tableManager.rollbackTransaction();
    }
}
```

## 数据类型支持

### 设置数据

```cpp
TableRecord record;

// 字符串
record.set("name", "张三");

// 整数
record.set("age", 25);

// 浮点数
record.set("salary", 5000.50);

// 布尔值
record.set("is_active", true);
```

### 获取数据

```cpp
// 获取字符串
std::string name = record.getString("name");

// 获取整数
int age = record.getInt("age");

// 获取浮点数
double salary = record.getDouble("salary");

// 获取布尔值
bool isActive = record.getBool("is_active");

// 检查字段是否存在
if (record.hasField("name"))
{
    // 字段存在
}
```

## 实际应用示例

### 用户管理系统

```cpp
class UserManager
{
private:
    TableManager& tableManager_;
    
public:
    UserManager(TableManager& tm) : tableManager_(tm) {}
    
    // 添加用户
    bool addUser(const std::string& name, const std::string& email, int age)
    {
        TableRecord user;
        user.set("name", name);
        user.set("email", email);
        user.set("age", age);
        user.set("is_active", true);
        
        return tableManager_.insert("users", user);
    }
    
    // 查找用户
    TableRecord findUserByEmail(const std::string& email)
    {
        return tableManager_.selectOne("users", "email = '" + email + "'");
    }
    
    // 更新用户状态
    bool activateUser(int userId)
    {
        TableRecord updateData;
        updateData.set("is_active", true);
        return tableManager_.updateById("users", updateData, "id", userId);
    }
    
    // 获取活跃用户列表
    std::vector<TableRecord> getActiveUsers()
    {
        return tableManager_.selectWhere("users", "is_active = 1");
    }
    
    // 删除非活跃用户
    bool deleteInactiveUsers()
    {
        return tableManager_.deleteWhere("users", "is_active = 0");
    }
};
```

### 产品管理系统

```cpp
class ProductManager
{
private:
    TableManager& tableManager_;
    
public:
    ProductManager(TableManager& tm) : tableManager_(tm) {}
    
    // 添加产品
    bool addProduct(const std::string& name, double price, int stock)
    {
        TableRecord product;
        product.set("name", name);
        product.set("price", price);
        product.set("stock", stock);
        product.set("created_at", "NOW()");
        
        return tableManager_.insert("products", product);
    }
    
    // 更新库存
    bool updateStock(int productId, int newStock)
    {
        TableRecord updateData;
        updateData.set("stock", newStock);
        return tableManager_.updateById("products", updateData, "id", productId);
    }
    
    // 查找库存不足的产品
    std::vector<TableRecord> getLowStockProducts(int threshold = 10)
    {
        return tableManager_.selectWhere("products", "stock < " + std::to_string(threshold));
    }
    
    // 获取产品总数
    int getProductCount()
    {
        return tableManager_.count("products");
    }
};
```

## 最佳实践

### 1. 错误处理

```cpp
// 检查操作是否成功
if (!tableManager.insert("users", user))
{
    std::cout << "插入失败: " << tableManager.getLastError() << std::endl;
    // 处理错误
}
```

### 2. 数据验证

```cpp
// 检查记录是否存在
auto user = tableManager.selectById("users", "id", userId);
if (user.hasField("name"))
{
    // 用户存在，可以继续操作
    std::string name = user.getString("name");
}
else
{
    // 用户不存在
    std::cout << "用户不存在" << std::endl;
}
```

### 3. 批量操作

```cpp
// 使用事务进行批量操作
if (tableManager.beginTransaction())
{
    bool success = true;
    
    for (const auto& user : users)
    {
        if (!tableManager.insert("users", user))
        {
            success = false;
            break;
        }
    }
    
    if (success)
    {
        tableManager.commitTransaction();
    }
    else
    {
        tableManager.rollbackTransaction();
    }
}
```

### 4. 性能优化

```cpp
// 只查询需要的字段
auto names = tableManager.select("users", {"name"});

// 使用条件查询减少数据传输
auto recentUsers = tableManager.selectWhere("users", "created_at > '2024-01-01'");

// 使用分页查询
auto query = tableManager.query()
    .select({"name", "email"})
    .from("users")
    .orderBy("created_at", false)
    .limit(20)
    .offset(40);  // 第3页
```

## 常见问题

### Q: 如何处理特殊字符？

A: TableManager会自动处理SQL注入防护，但建议在条件查询中使用参数化查询：

```cpp
// 推荐：使用selectOne进行精确查询
auto user = tableManager.selectOne("users", "email = 'user@example.com'");

// 避免：直接在条件中拼接用户输入
// auto user = tableManager.selectOne("users", "email = '" + userInput + "'");
```

### Q: 如何优化查询性能？

A: 
1. 只查询需要的字段
2. 使用适当的WHERE条件
3. 使用LIMIT限制结果集大小
4. 在数据库层面添加适当的索引

### Q: 如何处理大量数据？

A: 
1. 使用分页查询
2. 使用批量操作
3. 考虑使用流式处理
4. 在非高峰时段进行大批量操作

## 总结

通过使用`TableManager`，团队成员可以：

✅ **无需SQL知识** - 使用简单直观的API
✅ **类型安全** - 自动处理数据类型转换
✅ **防SQL注入** - 内置安全防护
✅ **事务支持** - 保证数据一致性
✅ **批量操作** - 提高性能
✅ **错误处理** - 完善的异常处理机制

这样，即使不熟悉SQL的团队成员也能安全、高效地操作数据库，专注于业务逻辑的实现。
