#include "network_lib/tcp_socket.h"
#include <vector>
#include "simple_protocol.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

void runTCPServer()
{
    network_lib::TCPSocket serverSocket;
    SimpleProtocol protocol;

    if (!serverSocket.bind(8888))
    {
        std::cerr << "Failed to bind TCP server" << std::endl;
        return;
    }

    if (!serverSocket.listen())
    {
        std::cerr << "Failed to listen TCP server" << std::endl;
        return;
    }

    std::cout << "TCP Server started on port 8888" << std::endl;

    // Accept connections and handle data
    for (int i = 0; i < 5; ++i)
    {
        if (serverSocket.waitForData(1000))
        {
            network_lib::TCPSocket* clientSocket = serverSocket.accept();
            if (clientSocket != nullptr)
            {
                std::cout << "Connection accepted" << std::endl;

                // Receive data
                uint8_t buffer[4096];
                size_t len = sizeof(buffer);
                if (clientSocket->receive(buffer, len))
                {
                    // Decode using protocol
                    std::vector<uint8_t> decoded = protocol.decode(buffer, len);
                    std::string message(reinterpret_cast<const char*>(decoded.data()), decoded.size());
                    std::cout << "Server received: " << message << std::endl;
                }

                delete clientSocket;
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    serverSocket.close();
    std::cout << "TCP Server stopped" << std::endl;
}

void runTCPClient()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    network_lib::TCPSocket socket;
    SimpleProtocol protocol;

    if (!socket.connect("127.0.0.1", 8888))
    {
        std::cerr << "Failed to connect to TCP server" << std::endl;
        return;
    }

    std::cout << "TCP Client connected to server" << std::endl;

    // Send messages
    std::string message = "Hello from TCP client!";

    // Encode using protocol
    std::vector<uint8_t> encoded = protocol.encode(message.c_str(), message.length());
    if (socket.send(encoded.data(), encoded.size()))
    {
        std::cout << "Client sent: " << message << std::endl;
    }

    socket.close();
    std::cout << "TCP Client disconnected" << std::endl;
}

int main()
{
    std::thread serverThread(runTCPServer);
    std::thread clientThread(runTCPClient);

    serverThread.join();
    clientThread.join();

    return 0;
}
