#include "network_lib/udp_socket.h"
#include "simple_protocol.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

void runUDPServer()
{
    network_lib::UDPSocket serverSocket;
    SimpleProtocol protocol;

    if (!serverSocket.bind(8889))
    {
        std::cerr << "Failed to bind UDP server" << std::endl;
        return;
    }

    std::cout << "UDP Server started on port 8889" << std::endl;

    // Receive data
    for (int i = 0; i < 5; ++i)
    {
        if (serverSocket.waitForData(1000))
        {
            uint8_t buffer[4096];
            size_t len = sizeof(buffer);
            std::string fromHost;
            int fromPort;

            if (serverSocket.receiveFrom(buffer, len, fromHost, fromPort))
            {
                // Decode using protocol
                std::vector<uint8_t> decoded = protocol.decode(buffer, len);
                if (!decoded.empty())
                {
                    std::string message(reinterpret_cast<const char*>(decoded.data()), decoded.size());
                    std::cout << "Server received from " << fromHost << ":" << fromPort
                              << ": " << message << std::endl;
                }
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    serverSocket.close();
    std::cout << "UDP Server stopped" << std::endl;
}

void runUDPClient()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    network_lib::UDPSocket socket;
    SimpleProtocol protocol;

    if (!socket.connect("127.0.0.1", 8889))
    {
        std::cerr << "Failed to connect to UDP server" << std::endl;
        return;
    }

    std::cout << "UDP Client connected to server" << std::endl;

    // Send messages
    std::string message = "Hello from UDP client!";
    
    // Encode using protocol
    std::vector<uint8_t> encoded = protocol.encode(message.c_str(), message.length());
    if (socket.send(encoded.data(), encoded.size()))
    {
        std::cout << "Client sent: " << message << std::endl;
    }

    socket.close();
    std::cout << "UDP Client disconnected" << std::endl;
}

int main()
{
    std::thread serverThread(runUDPServer);
    std::thread clientThread(runUDPClient);

    serverThread.join();
    clientThread.join();

    return 0;
}