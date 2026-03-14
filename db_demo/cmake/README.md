# CMake 配置文件

## 目录说明

此目录包含项目的 CMake 包配置文件，用于让其他项目能够找到和使用本项目。

## 文件说明

### db_utilConfig.cmake.in

包配置模板文件，用于生成 `db_utilConfig.cmake`。

**用途：**
- 允许其他项目通过 `find_package(db_util)` 找到本项目
- 导出所有层的目标（db_util, core, modules）
- 设置必要的包含目录和依赖

**使用示例：**

在其他 CMake 项目中：

```cmake
# CMakeLists.txt
find_package(db_util REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app 
    db_util::core           # DAO 层
    db_util::user_module    # 用户模块
    db_util::order_module   # 订单模块
)
```

## 位置说明

`cmake/` 目录位于项目根目录，而不是 `db_util/` 子目录中，因为：

1. **整体项目配置** - 这是整个分层架构项目的配置，不仅仅是基础设施层
2. **多层导出** - 需要导出 db_util、core、modules 等多个层次
3. **CMake 惯例** - 根目录的 cmake/ 用于项目级别的配置

## 相关文件

- 根目录 `CMakeLists.txt` - 主构建配置
- `db_util/CMakeLists.txt` - 基础设施层配置
- `core/CMakeLists.txt` - DAO层配置
- `modules/*/CMakeLists.txt` - 业务模块配置
