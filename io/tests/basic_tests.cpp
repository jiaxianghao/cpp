#include "TransportFactory.h"

#include <chrono>
#include <cstring>
#include <exception>
#include <iostream>
#include <semaphore.h>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <thread>
#include <unistd.h>

using namespace transport;

namespace
{

class ThreadJoinGuard
{
public:
    explicit ThreadJoinGuard(std::thread& thread)
        : m_thread(thread)
    {
    }

    ~ThreadJoinGuard()
    {
        if (m_thread.joinable())
        {
            m_thread.join();
        }
    }

private:
    std::thread& m_thread;
};

void Require(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

void TestFactoryTypeMismatch()
{
    SerialConfig serial_config;
    auto transport = TransportFactory::Create(TransportType::SOCKET, serial_config);
    Require(transport == nullptr, "Factory should reject mismatched config types");
}

void TestSocketRoundTrip()
{
    const int kPort = 40000 + (getpid() % 20000);
    const std::string payload = "ping over tcp";
    std::string received_by_server;
    std::exception_ptr server_failure;

    SocketConfig server_config;
    server_config.address = "127.0.0.1";
    server_config.port = kPort;
    server_config.protocol = SocketConfig::Protocol::TCP;
    server_config.is_server = true;

    SocketConfig client_config;
    client_config.address = "127.0.0.1";
    client_config.port = kPort;
    client_config.protocol = SocketConfig::Protocol::TCP;
    client_config.is_server = false;

    std::thread server_thread([&]()
    {
        try
        {
            auto server = TransportFactory::CreateSocket(server_config);
            Require(server->Open(), "Server failed to open: " + server->GetLastError());

            char buffer[128] = {};
            int received = server->Receive(buffer, sizeof(buffer));
            Require(received > 0, "Server failed to receive data: " + server->GetLastError());
            received_by_server.assign(buffer, static_cast<size_t>(received));

            int sent = server->Send(buffer, static_cast<size_t>(received));
            Require(sent == received, "Server failed to echo full payload");
            server->Close();
        }
        catch (...)
        {
            server_failure = std::current_exception();
        }
    });
    ThreadJoinGuard join_guard(server_thread);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto client = TransportFactory::CreateSocket(client_config);
    bool client_opened = false;
    for (int attempt = 0; attempt < 20 && !client_opened; ++attempt)
    {
        client_opened = client->Open();
        if (!client_opened)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    Require(client_opened, "Client failed to open: " + client->GetLastError());

    int sent = client->Send(payload.data(), payload.size());
    Require(sent == static_cast<int>(payload.size()), "Client failed to send full payload");

    char echo[128] = {};
    int received = client->Receive(echo, sizeof(echo));
    Require(received == static_cast<int>(payload.size()), "Client failed to receive echoed payload");
    Require(std::string(echo, echo + received) == payload, "Client received wrong echoed payload");
    client->Close();

    if (server_failure)
    {
        std::rethrow_exception(server_failure);
    }
    Require(received_by_server == payload, "Server received wrong payload");
}

void CleanupSharedMemoryArtifacts(const std::string& name)
{
    shm_unlink(name.c_str());
    sem_unlink((name + "_write").c_str());
    sem_unlink((name + "_read").c_str());
}

void TestSharedMemoryPersistsAfterCreatorClose()
{
    const std::string name = "/transport_test_shm";
    CleanupSharedMemoryArtifacts(name);

    SharedMemoryConfig writer_config;
    writer_config.name = name;
    writer_config.size = 128;
    writer_config.create = true;

    auto writer = TransportFactory::CreateSharedMemory(writer_config);
    Require(writer->Open(), "Writer failed to open shared memory: " + writer->GetLastError());

    const char payload[] = "shared memory payload";
    int sent = writer->Send(payload, sizeof(payload));
    Require(sent == static_cast<int>(sizeof(payload)), "Writer failed to write full shared memory payload");
    writer->Close();

    SharedMemoryConfig reader_config;
    reader_config.name = name;
    reader_config.size = 128;
    reader_config.create = false;

    auto reader = TransportFactory::CreateSharedMemory(reader_config);
    Require(reader->Open(), "Reader failed to attach shared memory after creator closed: " + reader->GetLastError());

    char buffer[128] = {};
    int received = reader->Receive(buffer, sizeof(buffer));
    Require(received == static_cast<int>(sizeof(payload)), "Reader failed to receive expected shared memory payload");
    Require(std::strcmp(buffer, payload) == 0, "Reader received wrong shared memory payload");
    reader->Close();

    CleanupSharedMemoryArtifacts(name);
}

} // namespace

int main()
{
    try
    {
        TestFactoryTypeMismatch();
        TestSocketRoundTrip();
        TestSharedMemoryPersistsAfterCreatorClose();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Test failure: " << ex.what() << std::endl;
        return 1;
    }

    std::cout << "All transport tests passed" << std::endl;
    return 0;
}
