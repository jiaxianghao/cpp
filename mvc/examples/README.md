# 使用示例

这个目录包含了三个不同层次的使用示例，展示如何在实际项目中使用分层架构。

## 📁 示例文件

### 1. `simple_usage.cpp` - 最基础的使用方法
**适用场景**: 初学者、简单项目
**特点**: 
- 最少的代码
- 最直接的使用方式
- 适合快速上手

```cpp
// 只需要这几行代码就能使用
auto user_controller = std::make_shared<UserController>(user_service);
auto result = user_controller->createUser("用户名", "邮箱");
```

### 2. `business_application.cpp` - 业务应用示例
**适用场景**: 实际业务系统、企业应用
**特点**:
- 完整的业务逻辑
- 错误处理和验证
- 用户友好的界面

```cpp
// 业务场景示例
BusinessApplication app;
app.registerUser("alice", "alice@example.com");
app.loginUser("alice");
app.updateUserProfile(1, "new@example.com");
```

### 3. `web_api_example.cpp` - Web API示例
**适用场景**: Web服务、RESTful API
**特点**:
- HTTP请求处理
- JSON响应
- RESTful接口设计

```cpp
// Web API示例
WebAPI api;
HttpResponse response = api.handleRequest(request);
```

## 🚀 如何运行示例

### 编译所有示例
```bash
cd examples
mkdir build && cd build
cmake ..
make
```

### 运行特定示例
```bash
# 简单使用示例
./bin/simple_usage

# 业务应用示例
./bin/business_application

# Web API示例
./bin/web_api_example
```

## 💡 使用建议

### 1. 如果你是初学者
- 从 `simple_usage.cpp` 开始
- 理解基本的API调用
- 逐步学习更复杂的功能

### 2. 如果你在开发业务系统
- 参考 `business_application.cpp`
- 学习错误处理和数据验证
- 了解完整的业务流程

### 3. 如果你在开发Web服务
- 参考 `web_api_example.cpp`
- 学习HTTP请求处理
- 了解RESTful API设计

## 🔧 自定义使用

### 基本使用模式
```cpp
// 1. 初始化系统
DatabaseConfig& config = DatabaseConfig::getInstance();
config.setHost("your_host");
config.setUser("your_user");
config.setPassword("your_password");
config.setDatabase("your_database");

// 2. 创建连接
auto connection = std::make_shared<DatabaseConnection>();
connection->connect();

// 3. 创建各层
auto user_repo = std::make_shared<UserRepository>(connection);
auto user_service = std::make_shared<UserService>(user_repo);
auto user_controller = std::make_shared<UserController>(user_service);

// 4. 使用系统
auto result = user_controller->createUser("username", "email");
```

### 常用操作
```cpp
// 创建用户
auto result = user_controller->createUser("用户名", "邮箱@example.com");

// 获取用户
User user = user_controller->getUserById(1);

// 搜索用户
auto users = user_controller->searchUsers("关键词");

// 更新用户
user.setEmail("new@example.com");
auto update_result = user_controller->updateUser(user);

// 删除用户
auto delete_result = user_controller->deleteUser(1);
```

## 📚 进阶使用

### 1. 错误处理
```cpp
auto result = user_controller->createUser("username", "email");
if (result.success) {
    // 处理成功情况
    std::cout << "用户创建成功: " << result.user.toString() << std::endl;
} else {
    // 处理错误情况
    std::cerr << "错误: " << result.error_message << std::endl;
}
```

### 2. 数据验证
```cpp
// 检查用户名是否可用
if (user_controller->isUsernameAvailable("username")) {
    // 用户名可用
}

// 检查邮箱是否可用
if (user_controller->isEmailAvailable("email@example.com")) {
    // 邮箱可用
}
```

### 3. 批量操作
```cpp
// 获取所有用户
auto all_users = user_controller->getAllUsers();
for (const auto& user : all_users) {
    std::cout << user.toString() << std::endl;
}

// 搜索用户
auto search_results = user_controller->searchUsers("关键词");
```

## ⚠️ 注意事项

1. **数据库连接**: 确保MySQL服务正在运行
2. **权限设置**: 确保数据库用户有足够的权限
3. **错误处理**: 始终检查操作结果
4. **资源管理**: 使用智能指针自动管理内存
5. **线程安全**: 在多线程环境中需要额外的同步机制

## 🎯 最佳实践

1. **初始化一次**: 在应用程序启动时初始化系统
2. **错误处理**: 始终检查操作结果并处理错误
3. **数据验证**: 在创建用户前验证数据
4. **日志记录**: 记录重要操作和错误
5. **资源清理**: 让智能指针自动管理资源
