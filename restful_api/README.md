# HTTP REST API 进程间通信示例

这个项目演示了如何使用HTTP REST API在两个C++应用程序之间进行通信。

## 项目结构

- `server.cpp` - HTTP服务器应用,提供API接口
- `client.cpp` - HTTP客户端应用,调用服务器接口
- `api_protocol.h` - 接口协议定义
- `httplib.h` - cpp-httplib库(header-only)

## 编译和运行

### 1. 编译项目
```bash
cd build
cmake ..
make
```

### 2. 启动服务器
```bash
./server
```
服务器将在端口8080上启动,提供以下接口:
- `GET /health` - 健康检查
- `POST /api/execute` - 执行命令

### 3. 运行客户端
在另一个终端中运行客户端:

```bash
# 检查服务器健康状态
./client health

# 获取字符串长度
./client length "Hello World"

# 转换为大写
./client uppercase "hello world"

# 反转字符串
./client reverse "Hello World"
```

## API接口说明

### POST /api/execute

**请求参数:**
- `command` (必需): 要执行的命令 (length, uppercase, reverse)
- `data` (可选): 要处理的数据

**响应格式:**
```json
{
  "status_code": 200,
  "success": true,
  "message": "Operation completed successfully",
  "result": "处理结果"
}
```

## 特性

- **简洁的接口**: 只需几行代码即可实现客户端调用
- **执行反馈**: 返回详细的状态码和执行结果
- **错误处理**: 完善的错误处理和状态反馈
- **跨平台**: 支持本地和网络通信
- **高性能**: 基于cpp-httplib,性能优异

## 扩展

您可以轻松扩展服务器端添加更多业务逻辑,客户端代码无需修改即可调用新功能。
