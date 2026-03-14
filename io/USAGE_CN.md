# 使用说明

## 快速开始

### 1. 构建项目

```bash
# 给构建脚本添加执行权限
chmod +x build.sh clean.sh

# 执行构建
./build.sh
```

### 2. 运行示例

```bash
cd build
./transport_example
```

## 使用方法

### 串口通信示例

```cpp
#include "TransportFactory.h"

// Create serial port configuration
SerialConfig config;
config.device = "/dev/ttyUSB0";
config.baudrate = 115200;
config.databits = 8;
config.stopbits = 1;
config.parity = 'N';

// Create transport instance
auto transport = TransportFactory::CreateSerial(config);

// Open connection
if (transport->Open())
{
    // Send data
    const char* data = "Hello";
    transport->Send(data, strlen(data));

    // Receive data
    char buffer[256];
    int received = transport->Receive(buffer, sizeof(buffer));
}
```

### Socket通信示例（TCP客户端）

```cpp
SocketConfig config;
config.address = "127.0.0.1";
config.port = 8080;
config.protocol = SocketConfig::Protocol::TCP;
config.is_server = false;

auto transport = TransportFactory::CreateSocket(config);
if (transport->Open())
{
    transport->Send("Hello Server", 12);
}
```

### Socket通信示例（TCP服务器）

```cpp
SocketConfig config;
config.address = "0.0.0.0";
config.port = 8080;
config.protocol = SocketConfig::Protocol::TCP;
config.is_server = true;

auto transport = TransportFactory::CreateSocket(config);
// Open() will accept one client connection
if (transport->Open())
{
    char buffer[256];
    int received = transport->Receive(buffer, sizeof(buffer));
}
```

### 共享内存通信示例

**写入进程：**

```cpp
SharedMemoryConfig config;
config.name = "/my_shm";
config.size = 4096;
config.create = true;  // Create new shared memory

auto transport = TransportFactory::CreateSharedMemory(config);
if (transport->Open())
{
    transport->Send("Hello", 5);
}
```

**读取进程：**

```cpp
SharedMemoryConfig config;
config.name = "/my_shm";
config.size = 4096;
config.create = false;  // Attach to existing

auto transport = TransportFactory::CreateSharedMemory(config);
if (transport->Open())
{
    char buffer[256];
    int received = transport->Receive(buffer, sizeof(buffer));
}
```

### CAN通信示例

```cpp
CanConfig config;
config.interface = "can0";
config.use_canfd = false;

auto transport = TransportFactory::CreateCan(config);
if (transport->Open())
{
    // Send CAN frame
    struct can_frame frame;
    frame.can_id = 0x123;
    frame.can_dlc = 8;
    for (int i = 0; i < 8; i++)
    {
        frame.data[i] = i;
    }
    transport->Send(&frame, sizeof(frame));

    // Receive CAN frame
    struct can_frame recv_frame;
    transport->Receive(&recv_frame, sizeof(recv_frame));
}
```

## 环境配置

### 串口权限

```bash
# 方法1：将用户添加到dialout组
sudo usermod -a -G dialout $USER
# 需要重新登录才能生效

# 方法2：直接修改设备权限
sudo chmod 666 /dev/ttyUSB0
```

### 虚拟CAN接口（用于测试）

```bash
# 加载vcan模块
sudo modprobe vcan

# 创建虚拟CAN接口
sudo ip link add dev vcan0 type vcan

# 启动接口
sudo ip link set up vcan0

# 验证
ip link show vcan0
```

### 物理CAN接口配置

```bash
# 配置CAN接口（波特率500k）
sudo ip link set can0 type can bitrate 500000

# 启动接口
sudo ip link set can0 up

# 查看状态
ip -details -statistics link show can0
```

### 共享内存清理

```bash
# 查看共享内存对象
ls -la /dev/shm/

# 删除共享内存对象
rm /dev/shm/my_shm

# 查看信号量
ls -la /dev/shm/sem.*

# 删除信号量
rm /dev/shm/sem.my_shm_*
```

## 错误处理

所有传输层都提供统一的错误处理：

```cpp
auto transport = TransportFactory::CreateSerial(config);

if (!transport->Open())
{
    // Get detailed error message
    std::cerr << "Error: " << transport->GetLastError() << std::endl;
    return -1;
}

int sent = transport->Send(data, size);
if (sent < 0)
{
    std::cerr << "Send failed: " << transport->GetLastError() << std::endl;
}

int received = transport->Receive(buffer, size);
if (received < 0)
{
    std::cerr << "Receive failed: " << transport->GetLastError() << std::endl;
}
```

## 线程安全

所有传输实现都是线程安全的，可以在多线程环境中使用：

```cpp
auto transport = TransportFactory::CreateSocket(config);
transport->Open();

// Thread 1: sending
std::thread sender([&transport]()
{
    transport->Send("data", 4);
});

// Thread 2: receiving
std::thread receiver([&transport]()
{
    char buffer[256];
    transport->Receive(buffer, sizeof(buffer));
});

sender.join();
receiver.join();
```

## 集成到你的项目

### 方法1：使用CMake子项目

```cmake
add_subdirectory(path/to/io)
target_link_libraries(your_app transport_static)
```

### 方法2：安装后使用

```bash
cd build
sudo make install
```

然后在你的CMakeLists.txt中：

```cmake
find_library(TRANSPORT_LIB transport)
target_link_libraries(your_app ${TRANSPORT_LIB} pthread rt)
```

### 方法3：直接包含源文件

将 `include/` 和 `src/` 目录复制到你的项目中，然后：

```cmake
include_directories(path/to/io/include)
add_library(transport
    path/to/io/src/TransportBase.cpp
    path/to/io/src/SerialTransport.cpp
    path/to/io/src/SocketTransport.cpp
    path/to/io/src/SharedMemoryTransport.cpp
    path/to/io/src/CanTransport.cpp
    path/to/io/src/TransportFactory.cpp
)
target_link_libraries(your_app transport pthread rt)
```

## 常见问题

### Q: 串口无法打开？
A: 检查设备路径是否正确，确认用户有权限访问设备（参见"串口权限"部分）。

### Q: CAN接口无法打开？
A: 确保CAN接口已启动，使用 `ip link show` 检查状态。

### Q: 共享内存无法创建？
A: 检查 `/dev/shm/` 是否有足够空间，确认权限正确。

### Q: Socket连接失败？
A: 确认目标地址和端口正确，检查防火墙设置。

### Q: 编译错误？
A: 确保安装了必要的开发工具和库（gcc, g++, cmake, pthread, rt）。

## 性能建议

1. **批量发送**：尽可能批量发送数据以减少系统调用开销
2. **缓冲区大小**：根据实际需求调整接收缓冲区大小
3. **串口波特率**：选择合适的波特率，更高的波特率不一定更好
4. **Socket**: TCP比UDP有更多开销，但提供可靠传输
5. **共享内存**：在进程间通信中性能最好，但需要额外的同步机制

## 调试技巧

### 串口调试

```bash
# 监控串口数据
cat /dev/ttyUSB0

# 或使用minicom
sudo minicom -D /dev/ttyUSB0 -b 115200
```

### CAN总线调试

```bash
# 安装can-utils
sudo apt-get install can-utils

# 监控CAN消息
candump vcan0

# 发送CAN消息
cansend vcan0 123#1122334455667788
```

### Socket调试

```bash
# 测试TCP服务器
nc -l 8080

# 测试TCP客户端
nc 127.0.0.1 8080

# 查看端口占用
netstat -tuln | grep 8080
```

### 共享内存调试

```bash
# 查看所有共享内存对象
ls -la /dev/shm/

# 查看共享内存统计
ipcs -m

# 查看信号量
ipcs -s
```
