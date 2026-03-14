#include "TransportFactory.h"
#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>

using namespace transport;

// Example: Serial Port Communication
void SerialExample()
{
    std::cout << "\n=== Serial Port Example ===" << std::endl;

    SerialConfig config;
    config.device = "/dev/ttyUSB0";  // Change to your serial device
    config.baudrate = 115200;
    config.databits = 8;
    config.stopbits = 1;
    config.parity = 'N';

    auto transport = TransportFactory::CreateSerial(config);

    if (!transport->Open())
    {
        std::cerr << "Failed to open serial port: " << transport->GetLastError() << std::endl;
        return;
    }

    std::cout << "Serial port opened successfully" << std::endl;

    // Send data
    const char* message = "Hello Serial!";
    int sent = transport->Send(message, std::strlen(message));
    if (sent > 0)
    {
        std::cout << "Sent " << sent << " bytes" << std::endl;
    }
    else
    {
        std::cerr << "Failed to send: " << transport->GetLastError() << std::endl;
    }

    // Receive data
    char buffer[256];
    int received = transport->Receive(buffer, sizeof(buffer) - 1);
    if (received > 0)
    {
        buffer[received] = '\0';
        std::cout << "Received " << received << " bytes: " << buffer << std::endl;
    }
    else if (received == 0)
    {
        std::cout << "No data received (timeout)" << std::endl;
    }
    else
    {
        std::cerr << "Failed to receive: " << transport->GetLastError() << std::endl;
    }

    transport->Close();
}

// Example: TCP Socket Communication (Client)
void SocketClientExample()
{
    std::cout << "\n=== TCP Socket Client Example ===" << std::endl;

    SocketConfig config;
    config.address = "127.0.0.1";
    config.port = 8080;
    config.protocol = SocketConfig::Protocol::TCP;
    config.is_server = false;

    auto transport = TransportFactory::CreateSocket(config);

    if (!transport->Open())
    {
        std::cerr << "Failed to connect to server: " << transport->GetLastError() << std::endl;
        return;
    }

    std::cout << "Connected to server" << std::endl;

    // Send data
    const char* message = "Hello Server!";
    int sent = transport->Send(message, std::strlen(message));
    if (sent > 0)
    {
        std::cout << "Sent " << sent << " bytes" << std::endl;
    }
    else
    {
        std::cerr << "Failed to send: " << transport->GetLastError() << std::endl;
    }

    // Receive data
    char buffer[256];
    int received = transport->Receive(buffer, sizeof(buffer) - 1);
    if (received > 0)
    {
        buffer[received] = '\0';
        std::cout << "Received " << received << " bytes: " << buffer << std::endl;
    }
    else
    {
        std::cerr << "Failed to receive: " << transport->GetLastError() << std::endl;
    }

    transport->Close();
}

// Example: TCP Socket Communication (Server)
void SocketServerExample()
{
    std::cout << "\n=== TCP Socket Server Example ===" << std::endl;

    SocketConfig config;
    config.address = "0.0.0.0";
    config.port = 8080;
    config.protocol = SocketConfig::Protocol::TCP;
    config.is_server = true;

    auto transport = TransportFactory::CreateSocket(config);

    std::cout << "Waiting for client connection..." << std::endl;

    if (!transport->Open())
    {
        std::cerr << "Failed to start server: " << transport->GetLastError() << std::endl;
        return;
    }

    std::cout << "Client connected" << std::endl;

    // Receive data
    char buffer[256];
    int received = transport->Receive(buffer, sizeof(buffer) - 1);
    if (received > 0)
    {
        buffer[received] = '\0';
        std::cout << "Received " << received << " bytes: " << buffer << std::endl;

        // Echo back
        int sent = transport->Send(buffer, received);
        if (sent > 0)
        {
            std::cout << "Echoed " << sent << " bytes back" << std::endl;
        }
    }
    else
    {
        std::cerr << "Failed to receive: " << transport->GetLastError() << std::endl;
    }

    transport->Close();
}

// Example: Shared Memory Communication
void SharedMemoryExample()
{
    std::cout << "\n=== Shared Memory Example ===" << std::endl;

    // Writer process
    {
        SharedMemoryConfig config;
        config.name = "/test_shm";
        config.size = 1024;
        config.create = true;

        auto transport = TransportFactory::CreateSharedMemory(config);

        if (!transport->Open())
        {
            std::cerr << "Failed to create shared memory: " << transport->GetLastError() << std::endl;
            return;
        }

        std::cout << "Shared memory created" << std::endl;

        // Send data
        const char* message = "Hello Shared Memory!";
        int sent = transport->Send(message, std::strlen(message) + 1);
        if (sent > 0)
        {
            std::cout << "Wrote " << sent << " bytes to shared memory" << std::endl;
        }
        else
        {
            std::cerr << "Failed to write: " << transport->GetLastError() << std::endl;
        }

        // Wait a bit for reader
        std::this_thread::sleep_for(std::chrono::seconds(2));

        transport->Close();
    }

    std::cout << "Writer finished, now trying to read..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Reader process (simulated in same program)
    {
        SharedMemoryConfig config;
        config.name = "/test_shm";
        config.size = 1024;
        config.create = false;

        auto transport = TransportFactory::CreateSharedMemory(config);

        if (!transport->Open())
        {
            std::cerr << "Failed to open shared memory: " << transport->GetLastError() << std::endl;
            return;
        }

        std::cout << "Shared memory opened" << std::endl;

        // Receive data
        char buffer[256];
        int received = transport->Receive(buffer, sizeof(buffer));
        if (received > 0)
        {
            std::cout << "Read " << received << " bytes from shared memory: " << buffer << std::endl;
        }
        else
        {
            std::cerr << "Failed to read: " << transport->GetLastError() << std::endl;
        }

        transport->Close();
    }
}

// Example: CAN Communication
void CanExample()
{
    std::cout << "\n=== CAN Example ===" << std::endl;

    CanConfig config;
    config.interface = "vcan0";  // Use virtual CAN interface for testing
    config.use_canfd = false;

    auto transport = TransportFactory::CreateCan(config);

    if (!transport->Open())
    {
        std::cerr << "Failed to open CAN interface: " << transport->GetLastError() << std::endl;
        std::cerr << "Note: You may need to set up a virtual CAN interface:" << std::endl;
        std::cerr << "  sudo modprobe vcan" << std::endl;
        std::cerr << "  sudo ip link add dev vcan0 type vcan" << std::endl;
        std::cerr << "  sudo ip link set up vcan0" << std::endl;
        return;
    }

    std::cout << "CAN interface opened successfully" << std::endl;

    // Send CAN frame
    struct can_frame frame;
    std::memset(&frame, 0, sizeof(frame));
    frame.can_id = 0x123;
    frame.can_dlc = 8;
    for (int i = 0; i < 8; i++)
    {
        frame.data[i] = i;
    }

    int sent = transport->Send(&frame, sizeof(frame));
    if (sent > 0)
    {
        std::cout << "Sent CAN frame with ID 0x" << std::hex << frame.can_id << std::dec << std::endl;
    }
    else
    {
        std::cerr << "Failed to send: " << transport->GetLastError() << std::endl;
    }

    // Receive CAN frame
    struct can_frame recv_frame;
    int received = transport->Receive(&recv_frame, sizeof(recv_frame));
    if (received > 0)
    {
        std::cout << "Received CAN frame with ID 0x" << std::hex << recv_frame.can_id << std::dec;
        std::cout << ", data:";
        for (int i = 0; i < recv_frame.can_dlc; i++)
        {
            std::cout << " 0x" << std::hex << static_cast<int>(recv_frame.data[i]) << std::dec;
        }
        std::cout << std::endl;
    }
    else if (received == 0)
    {
        std::cout << "No CAN frame received (timeout)" << std::endl;
    }
    else
    {
        std::cerr << "Failed to receive: " << transport->GetLastError() << std::endl;
    }

    transport->Close();
}

int main()
{
    std::cout << "Transport Layer Abstraction Examples" << std::endl;
    std::cout << "=====================================" << std::endl;

    // Note: Most examples will fail without proper hardware/setup
    // This demonstrates the unified interface usage

    // Uncomment the example you want to test:

    // SerialExample();
    // SocketClientExample();
    // SocketServerExample();
    // SharedMemoryExample();
    // CanExample();

    std::cout << "\nNote: All examples are commented out by default." << std::endl;
    std::cout << "Uncomment the example you want to test in main()." << std::endl;
    std::cout << "\nKey features demonstrated:" << std::endl;
    std::cout << "1. Unified interface across all transport types" << std::endl;
    std::cout << "2. Simple factory pattern for creating instances" << std::endl;
    std::cout << "3. Consistent error handling" << std::endl;
    std::cout << "4. RAII resource management" << std::endl;

    return 0;
}
