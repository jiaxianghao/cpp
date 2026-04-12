#include "event_loop.h"
#include "server.h"

#include <iostream>
#include <string>
#include <cstdint>

// ─────────────────────────────────────────────
// Echo 服务：将收到的数据原样回显给客户端
// ─────────────────────────────────────────────

static void echo_handler(TcpConnectionPtr conn, Buffer& buf) {
    // 取出当前所有可读数据
    std::string msg(buf.read_ptr(), buf.readable());
    buf.consume(buf.readable());

    std::cout << "[conn " << conn->id() << "] recv " << msg.size() << " bytes\n";

    // 异步回写
    conn->send(msg);
}

// ─────────────────────────────────────────────
// 主函数
// ─────────────────────────────────────────────

int main(int argc, char* argv[]) {
    const std::string host = "0.0.0.0";
    uint16_t          port = 9000;

    if (argc == 2) {
        port = static_cast<uint16_t>(std::stoi(argv[1]));
    } else if (argc == 3) {
        port = static_cast<uint16_t>(std::stoi(argv[2]));
    }

    try {
        EventLoop loop;
        TcpServer server(&loop, host, port);
        server.set_message_cb(echo_handler);
        server.start();

        std::cout << "[main] event loop running, press Ctrl+C to stop\n";
        loop.run();
        std::cout << "[main] event loop stopped\n";
    } catch (const std::exception& e) {
        std::cerr << "[fatal] " << e.what() << '\n';
        return 1;
    }

    return 0;
}
