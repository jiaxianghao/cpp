# C++ 数据库架构优化项目

这是一个展示如何优化C++项目中数据库访问架构的完整示例项目。项目采用分层架构模式，集成了现代C++开发的最佳实践。

## 🚀 主要特性

### ✅ 已实现的核心功能

1. **数据库连接池** - 高效管理数据库连接，提高性能
2. **Redis缓存层** - 内存缓存系统，减少数据库压力
3. **spdlog日志系统** - 结构化日志记录，支持多级别输出
4. **配置管理** - JSON配置文件，灵活管理应用参数
5. **Google Test单元测试** - 完整的测试覆盖
6. **RESTful API接口** - 现代化的HTTP API服务

### 🏗️ 架构设计

#### 分层架构

```
┌─────────────────────────────────────────────────────────────┐
│                    API Layer (RESTful)                      │
├─────────────────────────────────────────────────────────────┤
│                  Controllers Layer                          │
├─────────────────────────────────────────────────────────────┤
│                   Services Layer                            │
│              (Business Logic + Cache)                       │
├─────────────────────────────────────────────────────────────┤
│                Repositories Layer                           │
│              (Data Access Objects)                          │
├─────────────────────────────────────────────────────────────┤
│                 Database Layer                            │
│           (Connection Pool + ORM)                         │
├─────────────────────────────────────────────────────────────┤
│              Configuration & Logging                        │
└─────────────────────────────────────────────────────────────┘
```

#### 设计模式

- **单例模式**: 全局配置和连接管理
- **Repository模式**: 数据访问层封装
- **依赖注入**: 松耦合的组件设计
- **RAII**: 自动资源管理
- **缓存模式**: Cache-aside策略

## 📦 项目结构

```
project/
├── include/                    # 头文件目录
│   ├── api/                    # API层
│   │   ├── http_server.h       # HTTP服务器
│   │   └── rest_api_controller.h # REST控制器
│   ├── cache/                  # 缓存层
│   │   ├── cache_manager.h     # 缓存管理器
│   │   └── cache_service.h     # 缓存服务
│   ├── database/               # 数据库层
│   │   ├── database_config.h   # 数据库配置
│   │   ├── database_connection.h # 数据库连接
│   │   └── database_connection_pool.h # 连接池
│   ├── models/                 # 数据模型
│   │   ├── user.h             # 用户模型
│   │   ├── product.h          # 产品模型
│   │   └── order.h            # 订单模型
│   ├── repositories/          # 数据访问层
│   │   ├── user_repository.h  # 用户仓库
│   │   ├── product_repository.h # 产品仓库
│   │   └── order_repository.h  # 订单仓库
│   ├── services/              # 业务逻辑层
│   │   ├── user_service.h     # 用户服务
│   │   ├── product_service.h   # 产品服务
│   │   ├── order_service.h    # 订单服务
│   │   └── application_service.h # 应用服务
│   ├── controllers/           # 控制器层
│   │   ├── user_controller.h   # 用户控制器
│   │   ├── product_controller.h # 产品控制器
│   │   └── order_controller.h  # 订单控制器
│   └── utils/                 # 工具类
│       ├── logger.h           # 日志管理
│       └── config_manager.h   # 配置管理
├── src/                       # 源文件目录
│   ├── api/
│   ├── cache/
│   ├── database/
│   ├── models/
│   ├── repositories/
│   ├── services/
│   ├── controllers/
│   └── utils/
├── tests/                     # 单元测试
│   ├── test_main.cpp
│   ├── test_database_config.cpp
│   ├── test_cache_manager.cpp
│   └── test_user_service.cpp
├── examples/                  # 示例代码
├── config.json               # 配置文件
├── CMakeLists.txt            # CMake构建文件
├── main.cpp                  # 主程序
├── api_server.cpp           # API服务器
├── setup_database.sql       # 数据库初始化脚本
└── README.md               # 说明文档
```

## 🛠️ 编译和运行

### 前置要求

- C++17 编译器 (GCC 7+ 或 Clang 5+)
- CMake 3.10+
- MySQL开发库
- MySQL服务器

### 安装依赖

#### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libmysqlclient-dev mysql-server
```

#### CentOS/RHEL:
```bash
sudo yum install gcc-c++ cmake mysql-devel mysql-server
```

### 编译项目

```bash
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake ..

# 编译
make -j$(nproc)

# 运行主程序
./bin/DatabaseArchitectureDemo

# 运行API服务器
./bin/DatabaseArchitectureDemo_api --config ../config.json --port 8080
```

### 运行测试

```bash
# 运行单元测试
make test
# 或者
./tests/DatabaseArchitectureDemo_tests
```

## ⚙️ 配置管理

项目使用JSON配置文件管理所有参数：

```json
{
  "database": {
    "host": "localhost",
    "port": 3306,
    "user": "root",
    "password": "password",
    "database": "myapp",
    "max_connections": 20,
    "min_connections": 5,
    "connection_timeout": 30,
    "auto_reconnect": true
  },
  "cache": {
    "host": "localhost",
    "port": 6379,
    "database": 0,
    "password": "",
    "connection_timeout": 5,
    "socket_timeout": 5,
    "pool_size": 10
  },
  "log": {
    "level": "info",
    "file": "logs/app.log",
    "max_file_size": 5242880,
    "max_files": 3,
    "console_output": true,
    "file_output": true
  },
  "server": {
    "host": "0.0.0.0",
    "port": 8080,
    "thread_pool_size": 4,
    "request_timeout": 30,
    "enable_ssl": false,
    "ssl_cert_file": "",
    "ssl_key_file": ""
  }
}
```

## 🌐 RESTful API 端点

### 用户管理API

| 方法 | 端点 | 描述 |
|------|------|------|
| GET | `/api/health` | 健康检查 |
| GET | `/api/info` | API信息 |
| GET | `/api/users` | 获取所有用户 |
| GET | `/api/users/:id` | 根据ID获取用户 |
| POST | `/api/users` | 创建新用户 |
| PUT | `/api/users/:id` | 更新用户 |
| DELETE | `/api/users/:id` | 删除用户 |
| GET | `/api/users/search?q=<query>` | 搜索用户 |
| GET | `/api/users/check-username/:username` | 检查用户名可用性 |
| GET | `/api/users/check-email/:email` | 检查邮箱可用性 |
| GET | `/api/users/stats` | 获取用户统计 |

### 示例API调用

```bash
# 健康检查
curl http://localhost:8080/api/health

# 获取所有用户
curl http://localhost:8080/api/users

# 创建新用户
curl -X POST -H "Content-Type: application/json" \
  -d '{"username":"john_doe","email":"john@example.com"}' \
  http://localhost:8080/api/users

# 搜索用户
curl "http://localhost:8080/api/users/search?q=john"

# 检查用户名可用性
curl http://localhost:8080/api/users/check-username/john_doe
```

## 🔧 核心组件详解

### 1. 数据库连接池

- **自动连接管理**: 动态创建和回收连接
- **连接验证**: 定期检测连接有效性
- **超时处理**: 防止连接泄漏
- **统计监控**: 实时连接池状态

```cpp
auto& pool = DatabaseConnectionPool::getInstance();
pool.initialize(5, 20, std::chrono::seconds(30));
auto conn = pool.getConnection(); // RAII自动管理
```

### 2. Redis缓存层

- **多种数据结构**: 字符串、哈希、列表、集合
- **TTL支持**: 自动过期机制
- **批量操作**: 高效的数据处理
- **统计信息**: 命中率监控

```cpp
auto& cache = CacheManager::getInstance();
cache.set("user:1", user_data, std::chrono::seconds(1800));
auto cached_data = cache.get("user:1");
```

### 3. 日志系统

- **多级别日志**: trace, debug, info, warn, error, critical
- **多种输出**: 控制台 + 文件 + 轮转
- **格式化**: 结构化日志输出
- **高性能**: 异步日志记录

```cpp
Logger::info("User created: {}", username);
Logger::error("Database connection failed: {}", error);
```

### 4. 配置管理

- **JSON格式**: 易于编辑和维护
- **热重载**: 运行时配置更新
- **类型安全**: 自动类型转换和验证
- **默认值**: 智能默认配置

```cpp
auto& config = ConfigManager::getInstance();
config.loadConfig("config.json");
auto db_config = config.getDatabaseConfig();
```

## 🧪 测试

项目包含完整的单元测试套件：

### 数据库配置测试
- 默认配置验证
- 配置文件加载
- 参数设置和获取

### 缓存管理器测试
- 基本CRUD操作
- 过期机制测试
- 批量操作验证
- 统计信息测试

### 用户服务测试
- 用户创建和验证
- 缓存集成测试
- 业务逻辑验证
- 错误处理测试

## 📊 性能优化

### 数据库优化
- ✅ 连接池减少连接开销
- ✅ 预处理语句防止SQL注入
- ✅ 索引优化查询性能
- ✅ 事务管理保证数据一致性

### 缓存优化
- ✅ Cache-aside模式
- ✅ 智能缓存失效策略
- ✅ 批量操作减少网络开销
- ✅ 内存使用优化

### 应用优化
- ✅ 异步日志记录
- ✅ 线程池处理并发请求
- ✅ 智能内存管理
- ✅ 编译优化选项

## 🔒 安全特性

- **SQL注入防护**: 参数化查询
- **输入验证**: 严格的数据验证
- **错误处理**: 安全的错误信息
- **日志安全**: 敏感信息脱敏
- **配置安全**: 配置文件加密支持

## 🚀 部署建议

### 生产环境配置

1. **数据库配置**
   - 使用专用数据库用户
   - 启用SSL连接
   - 配置适当的连接池大小
   - 设置合理的超时时间

2. **缓存配置**
   - 部署Redis集群
   - 配置持久化策略
   - 设置内存限制
   - 启用认证

3. **日志配置**
   - 配置日志轮转
   - 设置适当的日志级别
   - 集中化日志收集
   - 监控日志异常

4. **API配置**
   - 启用HTTPS
   - 配置限流
   - 设置超时时间
   - 启用监控

## 📈 监控和运维

### 关键指标

- **数据库连接池**: 连接数、等待时间、错误率
- **缓存性能**: 命中率、响应时间、内存使用
- **API性能**: 请求量、响应时间、错误率
- **系统资源**: CPU、内存、磁盘、网络

### 健康检查

```bash
# API健康检查
curl http://localhost:8080/api/health

# 数据库连接检查
curl http://localhost:8080/api/app/stats

# 缓存状态检查
curl http://localhost:8080/api/app/cache/stats
```

## 🤝 贡献指南

1. Fork项目
2. 创建特性分支 (`git checkout -b feature/amazing-feature`)
3. 提交更改 (`git commit -m 'Add some amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 创建Pull Request

## 📄 许可证

本项目采用MIT许可证 - 详见LICENSE文件

## 🙏 致谢

- [spdlog](https://github.com/gabime/spdlog) - 快速C++日志库
- [Google Test](https://github.com/google/googletest) - C++测试框架
- [MySQL](https://www.mysql.com/) - 关系型数据库
- [CMake](https://cmake.org/) - 构建系统

---

**⭐ 如果这个项目对您有帮助，请给它一个星标！**