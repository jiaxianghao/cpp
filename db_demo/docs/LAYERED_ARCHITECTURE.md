# 分层架构说明

## 架构概述

本项目采用经典的分层架构设计，将数据访问、业务逻辑和应用层分离，提供了清晰的职责划分和良好的可维护性。

## 架构图

```
┌─────────────────────────────────────────┐
│       Application Layer (app/)          │  应用程序入口
│         - main.cpp                      │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│    Business Logic Layer (modules/)      │  业务逻辑层
│         - user_module/                  │
│         - order_module/                 │
│         - product_module/               │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│       Core/DAO Layer (core/)            │  核心数据访问层
│         - entities/                     │  实体类
│         - dao/                          │  数据访问对象
│         - DatabaseService               │  数据库服务单例
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│    Infrastructure Layer (db_util/)      │  基础设施层
│         - DatabaseManager               │  数据库管理器
│         - TableManager                  │  表管理器
└────────────────┬────────────────────────┘
                 │
                 ▼
            MySQL Database
```

## 各层职责

### 1. Infrastructure Layer（基础设施层）- db_util/

**职责：**
- 提供底层数据库连接管理
- 封装 MySQL Connector/C++ 的基本操作
- 提供通用的数据库操作接口（SQL执行、事务管理等）

**包含组件：**
- `DatabaseManager`: 数据库连接和基本SQL操作
- `TableManager`: 表级别的CRUD操作封装

**使用场景：**
- 其他层不应直接使用此层，而应通过 Core 层访问
- 仅在需要执行特殊SQL操作时才直接访问

### 2. Core/DAO Layer（核心数据访问层）- core/

**职责：**
- 定义业务实体（Entity）
- 提供针对每个实体的数据访问对象（DAO）
- 管理数据库连接和DAO实例（DatabaseService单例）
- 将数据库记录转换为业务实体

**包含组件：**

#### Entities（实体类）
- `User`: 用户实体
- `Order`: 订单实体
- `Product`: 产品实体

#### DAOs（数据访问对象）
- `UserDAO`: 用户数据访问
- `OrderDAO`: 订单数据访问
- `ProductDAO`: 产品数据访问

#### Service
- `DatabaseService`: 单例模式，管理数据库连接和所有DAO实例

**特点：**
- 类型安全：使用业务实体而非通用记录
- 封装SQL：业务代码无需编写SQL
- 统一管理：所有数据访问通过DAO进行

### 3. Business Logic Layer（业务逻辑层）- modules/

**职责：**
- 实现具体的业务逻辑
- 调用DAO层进行数据操作
- 提供业务级别的API
- 处理业务验证和流程控制

**包含模块：**
- `UserModule`: 用户相关业务（注册、查询、更新等）
- `OrderModule`: 订单相关业务（创建订单、完成订单、查询等）
- `ProductModule`: 产品相关业务（添加、更新、库存管理等）

**特点：**
- 只依赖 Core 层，不直接访问 Infrastructure 层
- 关注业务逻辑，不关心数据存储细节
- 可以组合多个DAO完成复杂业务

### 4. Application Layer（应用层）- app/

**职责：**
- 应用程序入口点
- 初始化各个层次
- 协调各个业务模块
- 处理用户交互（如果是GUI/CLI应用）

**包含文件：**
- `main.cpp`: 主程序入口

## 数据流示例

### 用户注册流程

```
1. Application (main.cpp)
   └─> 调用 UserModule.registerUser()

2. Business Logic (UserModule)
   ├─> 业务验证（年龄、邮箱格式等）
   ├─> 调用 UserDAO.findByEmail() 检查邮箱是否已存在
   └─> 调用 UserDAO.create() 创建用户

3. DAO Layer (UserDAO)
   ├─> 将 User 实体转换为 TableRecord
   └─> 调用 TableManager.insert()

4. Infrastructure (TableManager)
   ├─> 构建 INSERT SQL 语句
   └─> 调用 DatabaseManager.executeQuery()

5. Infrastructure (DatabaseManager)
   └─> 执行 MySQL 操作

6. MySQL Database
   └─> 存储数据
```

## 依赖关系

```
app/
 └─> modules/
      └─> core/
           └─> db_util/
                └─> MySQL Connector/C++
```

**依赖规则：**
- 上层可以依赖下层
- 下层不应依赖上层
- 同层之间可以相互调用（如不同的业务模块）

## 优势总结

### 1. 职责分离
- 每一层有明确的职责
- 代码组织清晰，易于理解

### 2. 易于维护
- 数据库变更只需修改 DAO 层
- 业务逻辑变更只需修改对应的 Module
- 修改影响范围可控

### 3. 代码复用
- 多个业务模块可以共享同一套 DAO
- 通用功能集中在 Core 层

### 4. 易于测试
- 可以 mock DAO 层进行业务逻辑测试
- 可以单独测试每一层

### 5. 团队协作
- 不同技能水平的开发者可以在不同层工作
- 熟悉SQL的开发者维护 DAO 层
- 不熟悉SQL的开发者专注业务逻辑

### 6. 类型安全
- 使用业务实体（User、Order）而不是通用的 TableRecord
- 编译时类型检查

### 7. 统一管理
- DatabaseService 单例统一管理数据库连接
- 避免重复创建连接

## 扩展指南

### 添加新实体

1. 在 `core/include/entities/` 创建实体类
2. 在 `core/src/entities/` 实现实体类
3. 在 `core/include/dao/` 创建 DAO 类
4. 在 `core/src/dao/` 实现 DAO 类
5. 在 `DatabaseService` 中添加 DAO 实例和 getter
6. 在 `DatabaseService::initializeTables()` 中创建表

### 添加新业务模块

1. 在 `modules/` 下创建新目录
2. 创建头文件和实现文件
3. 创建 CMakeLists.txt
4. 在根 CMakeLists.txt 中添加子目录
5. 在业务代码中通过 `DatabaseService::getInstance()` 获取所需的 DAO

## 最佳实践

### DO（推荐）

✅ 业务模块只通过 DAO 访问数据库  
✅ 使用业务实体而非 TableRecord 传递数据  
✅ 在 DAO 层进行数据验证  
✅ 在业务层进行业务规则验证  
✅ 使用事务处理需要原子性的操作  
✅ 为复杂查询提供专门的 DAO 方法  

### DON'T（不推荐）

❌ 在业务模块中直接使用 DatabaseManager  
❌ 在业务模块中编写 SQL 语句  
❌ 跨层调用（如 App 直接调用 DAO）  
❌ 在 DAO 中包含业务逻辑  
❌ 硬编码数据库连接信息  
❌ 忽略错误处理  

## 参考文献

- [Martin Fowler - Patterns of Enterprise Application Architecture](https://martinfowler.com/eaaCatalog/)
- [Repository Pattern](https://martinfowler.com/eaaCatalog/repository.html)
- [Data Access Object Pattern](https://www.oracle.com/java/technologies/dataaccessobject.html)
