# 更新日志

## [2.0.0] - 2026-01-16

### 重大变更 🔥
- **目录结构重组** - 将 db_util 基础设施层移动到独立的 `db_util/` 目录
  - 原 `include/` → `db_util/include/`
  - 原 `src/` → `db_util/src/`
  - 原 `examples/` → `db_util/examples/`
  - 原 `tests/` → `db_util/tests/`

### 新增功能 ✨
- **分层架构实现** - 完整的四层架构设计
  - Infrastructure Layer (db_util/)
  - Data Access Layer (core/)
  - Business Logic Layer (modules/)
  - Application Layer (app/)

- **Core 模块** - 核心数据访问层
  - 实体类：User, Order, Product
  - DAO：UserDAO, OrderDAO, ProductDAO
  - DatabaseService 单例服务

- **Business Modules** - 业务逻辑模块
  - UserModule - 用户业务逻辑
  - OrderModule - 订单业务逻辑

- **Application** - 演示应用程序
  - 完整的使用示例
  - 展示跨模块操作
  - 事务处理演示

### 文档更新 📚
- 新增 `LAYERED_ARCHITECTURE.md` - 详细架构说明
- 新增 `QUICK_START.md` - 5分钟快速入门
- 新增 `BUILD_AND_RUN.md` - 构建运行指南
- 新增 `DIRECTORY_STRUCTURE.md` - 目录结构说明
- 新增 `ARCHITECTURE_DIAGRAM.txt` - 架构图示
- 更新 `README.md` - 反映新的项目结构
- 新增 `PROJECT_STRUCTURE.txt` - 完整目录树
- 新增 `.gitignore` - Git 配置文件

### 构建系统 🔧
- 模块化的 CMakeLists.txt 配置
  - 每个层次独立的构建配置
  - 清晰的依赖关系管理
  - 可选的示例和测试构建

### 改进 🚀
- **更清晰的职责划分** - 每一层都有明确的职责
- **更好的代码组织** - 目录结构反映架构设计
- **更易于扩展** - 模块化设计便于添加新功能
- **更友好的团队协作** - 不同技能开发者可在不同层工作

## [1.0.0] - 2026-01-15 (之前的版本)

### 初始功能
- DatabaseManager - 数据库连接管理
- TableManager - 表操作封装
- 基本的 CRUD 操作
- 事务支持
- 预处理语句
- 示例程序

---

## 迁移指南

### 从 1.0.0 升级到 2.0.0

#### 1. 更新包含路径

**之前：**
```cpp
#include "DatabaseManager.h"
#include "TableManager.h"
```

**现在（如果直接使用 db_util）：**
```cpp
// 仍然相同，CMake 已配置好路径
#include "DatabaseManager.h"
#include "TableManager.h"
```

**推荐使用方式（通过 core 层）：**
```cpp
#include "DatabaseService.h"
#include "entities/User.h"
#include "dao/UserDAO.h"
```

#### 2. 更新 CMakeLists.txt

**之前：**
```cmake
add_executable(my_app main.cpp)
target_link_libraries(my_app db_util)
```

**现在：**
```cmake
# 使用分层架构
add_executable(my_app main.cpp)
target_link_libraries(my_app 
    user_module      # 业务模块
    order_module     # 业务模块
    core             # DAO 层
    db_util          # 基础设施
)
```

#### 3. 代码迁移建议

**老方式（仍然可用）：**
```cpp
DatabaseManager dbManager;
dbManager.connect(...);
TableManager tm(dbManager);
tm.insert("users", record);
```

**新方式（推荐）：**
```cpp
// 初始化
auto& dbService = core::DatabaseService::getInstance();
dbService.initialize(...);

// 使用业务模块
modules::UserModule userModule;
userModule.registerUser("张三", "zhang@example.com", 25);

// 或直接使用 DAO
auto& userDAO = dbService.getUserDAO();
core::entities::User user("张三", "zhang@example.com", 25);
userDAO.create(user);
```

#### 4. 目录结构变化

确保你的代码引用了正确的路径：
- `include/DatabaseManager.h` → `db_util/include/DatabaseManager.h`
- `src/DatabaseManager.cpp` → `db_util/src/DatabaseManager.cpp`
- `examples/` → `db_util/examples/`
- `tests/` → `db_util/tests/`

#### 5. 重新编译

```bash
# 清理旧的构建
rm -rf build

# 重新构建
mkdir build && cd build
cmake ..
make -j$(nproc)
```

---

**说明：**
- 版本号遵循语义化版本规范 (Semantic Versioning)
- MAJOR.MINOR.PATCH (主版本号.次版本号.补丁版本号)
- 重大不兼容变更增加主版本号
