# 目录结构说明

## 📂 重组后的项目结构

项目现在采用清晰的分层目录结构，每一层都有独立的目录：

```
db_util_project/                        # 项目根目录
│
├── 📁 db_util/                         # 基础设施层 (Infrastructure Layer)
│   ├── include/                        # 公共头文件
│   │   ├── DatabaseManager.h           # 数据库连接管理器
│   │   └── TableManager.h              # 表操作管理器
│   ├── src/                            # 源文件实现
│   │   ├── DatabaseManager.cpp
│   │   └── TableManager.cpp
│   ├── examples/                       # 使用示例
│   │   ├── easy_usage_example.cpp
│   │   ├── simple_db_example.cpp
│   │   └── simple_table_example.cpp
│   ├── tests/                          # 单元测试
│   │   └── debug_test.cpp
│   └── CMakeLists.txt                  # 独立的构建配置
│
├── 📁 core/                            # 核心DAO层 (Data Access Layer)
│   ├── include/
│   │   ├── entities/                   # 业务实体定义
│   │   │   ├── User.h
│   │   │   ├── Order.h
│   │   │   └── Product.h
│   │   ├── dao/                        # 数据访问对象
│   │   │   ├── UserDAO.h
│   │   │   ├── OrderDAO.h
│   │   │   └── ProductDAO.h
│   │   └── DatabaseService.h           # 数据库服务单例
│   ├── src/
│   │   ├── entities/
│   │   ├── dao/
│   │   └── DatabaseService.cpp
│   └── CMakeLists.txt
│
├── 📁 modules/                         # 业务逻辑层 (Business Logic Layer)
│   ├── user_module/
│   │   ├── include/UserModule.h
│   │   ├── src/UserModule.cpp
│   │   └── CMakeLists.txt
│   └── order_module/
│       ├── include/OrderModule.h
│       ├── src/OrderModule.cpp
│       └── CMakeLists.txt
│
├── 📁 app/                             # 应用层 (Application Layer)
│   ├── main.cpp
│   └── CMakeLists.txt
│
├── 📁 docs/                            # 文档目录
│   ├── LAYERED_ARCHITECTURE.md
│   ├── BUILD_AND_RUN.md
│   ├── QUICK_START.md
│   ├── DIRECTORY_STRUCTURE.md          # 本文件
│   └── ...
│
├── 📁 cmake/                           # CMake 包配置文件
│   ├── db_utilConfig.cmake.in          # 包配置模板（用于 find_package）
│   └── README.md                       # cmake 目录说明
│
├── CMakeLists.txt                      # 根构建配置
├── README.md                           # 项目主文档
├── PROJECT_STRUCTURE.txt               # 详细结构说明
└── .gitignore                          # Git 忽略配置
```

## 🎯 重组的目的

### 之前的问题：
❌ 基础设施层的代码（include/, src/, examples/, tests/）散落在根目录  
❌ 目录结构不够清晰，难以区分各层职责  
❌ 新加入的开发者可能混淆各个层次  

### 重组后的优势：
✅ **更清晰的层次划分** - 每一层都有独立的目录  
✅ **更好的模块化** - db_util 作为独立的基础库  
✅ **更易于理解** - 目录结构直观反映架构设计  
✅ **更易于维护** - 各层代码完全分离  
✅ **更易于扩展** - 添加新层或新模块时结构明确  

## 📊 各目录职责

### db_util/ - 基础设施层
**职责：** 提供底层数据库操作能力
- 封装 MySQL Connector/C++
- 提供数据库连接管理
- 提供基本的 CRUD 操作
- **不包含业务逻辑**

**依赖：** 
- MySQL Connector/C++
- 无内部依赖

**被依赖：**
- core 层使用

### core/ - 核心DAO层
**职责：** 提供类型安全的数据访问
- 定义业务实体（Entity）
- 提供数据访问对象（DAO）
- 管理数据库服务（DatabaseService）
- 将数据库记录转换为业务对象

**依赖：**
- db_util 层

**被依赖：**
- modules 层使用

### modules/ - 业务逻辑层
**职责：** 实现具体业务逻辑
- 处理业务规则
- 组合多个 DAO 完成复杂操作
- 提供业务级别的 API
- **不直接操作数据库**

**依赖：**
- core 层

**被依赖：**
- app 层使用

### app/ - 应用层
**职责：** 应用程序入口
- 初始化各个层次
- 协调业务模块
- 处理用户交互

**依赖：**
- modules 层
- core 层

## 🔄 构建依赖关系

```
CMakeLists.txt (root)
    ├─→ add_subdirectory(db_util)          # 先构建基础设施
    ├─→ add_subdirectory(core)             # 依赖 db_util
    ├─→ add_subdirectory(modules/user_module)    # 依赖 core
    ├─→ add_subdirectory(modules/order_module)   # 依赖 core
    └─→ add_subdirectory(app)              # 依赖 modules
```

每个子目录都有自己的 CMakeLists.txt，负责：
- 定义该模块的源文件
- 指定依赖关系
- 设置包含路径
- 配置编译选项

## 📝 路径引用规范

### 在 core/ 中引用 db_util
```cpp
#include "DatabaseManager.h"  // 来自 db_util/include/
#include "TableManager.h"     // 来自 db_util/include/
```

### 在 modules/ 中引用 core
```cpp
#include "entities/User.h"    // 来自 core/include/entities/
#include "dao/UserDAO.h"      // 来自 core/include/dao/
#include "DatabaseService.h"  // 来自 core/include/
```

### 在 app/ 中引用 modules
```cpp
#include "UserModule.h"       // 来自 modules/user_module/include/
#include "OrderModule.h"      // 来自 modules/order_module/include/
```

## 🛠️ 添加新模块的步骤

### 1. 在 db_util/ 添加新功能
```bash
# 添加新的基础设施功能
db_util/include/NewManager.h
db_util/src/NewManager.cpp
# 更新 db_util/CMakeLists.txt
```

### 2. 在 core/ 添加新实体和 DAO
```bash
# 添加新实体
core/include/entities/NewEntity.h
core/src/entities/NewEntity.cpp

# 添加新 DAO
core/include/dao/NewDAO.h
core/src/dao/NewDAO.cpp

# 在 DatabaseService 中注册
# 更新 core/CMakeLists.txt
```

### 3. 在 modules/ 添加新业务模块
```bash
# 创建新模块目录
mkdir -p modules/new_module/include
mkdir -p modules/new_module/src

# 创建模块文件
modules/new_module/include/NewModule.h
modules/new_module/src/NewModule.cpp
modules/new_module/CMakeLists.txt

# 在根 CMakeLists.txt 添加：
# add_subdirectory(modules/new_module)
```

## 🔍 快速查找文件

| 想要查找... | 在这个目录 | 示例 |
|-----------|----------|------|
| 数据库连接相关 | `db_util/include/` | DatabaseManager.h |
| 基础数据库操作 | `db_util/src/` | TableManager.cpp |
| 实体定义 | `core/include/entities/` | User.h |
| DAO 接口 | `core/include/dao/` | UserDAO.h |
| DAO 实现 | `core/src/dao/` | UserDAO.cpp |
| 业务逻辑接口 | `modules/*/include/` | UserModule.h |
| 业务逻辑实现 | `modules/*/src/` | UserModule.cpp |
| 程序入口 | `app/` | main.cpp |
| 使用示例 | `db_util/examples/` | easy_usage_example.cpp |
| 测试代码 | `db_util/tests/` | debug_test.cpp |
| 文档 | `docs/` | *.md |

## 📚 相关文档

- [README.md](../README.md) - 项目概述
- [LAYERED_ARCHITECTURE.md](LAYERED_ARCHITECTURE.md) - 架构设计详解
- [QUICK_START.md](QUICK_START.md) - 快速入门指南
- [BUILD_AND_RUN.md](BUILD_AND_RUN.md) - 构建和运行指南
- [PROJECT_STRUCTURE.txt](../PROJECT_STRUCTURE.txt) - 详细的目录树

---

**最后更新：** 2026-01-16  
**版本：** 2.0 (重组后)
