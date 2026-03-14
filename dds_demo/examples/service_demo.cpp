// Service demo: AddTwoInts
// Demonstrates ROS-style request/response services built on top of DDS.
//
// The demo runs a ServiceServer and two ServiceClients in the same process:
//   - One client uses synchronous call()
//   - One client uses asynchronous callAsync()

#include "dds_wrapper/DDSManager.h"
#include "IDLTypes.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

using namespace dds_wrapper;

// Pretty-print helper
static std::mutex print_mutex;

template<typename... Args>
static void println(Args&&... args)
{
    std::lock_guard<std::mutex> lock(print_mutex);
    (std::cout << ... << std::forward<Args>(args));
    std::cout << '\n';
}

int main()
{
    try
    {
        DDSManager manager;
        if (!manager.initialize("../../config/default_config.json"))
        {
            std::cerr << "Failed to initialize DDSManager\n";
            return 1;
        }
        println("DDSManager initialized");

        // ------------------------------------------------------------------ //
        // Service Server
        // ------------------------------------------------------------------ //
        auto server = manager.createServiceServer<AddTwoIntsRequest, AddTwoIntsResponse>(
            "add_two_ints",
            [](const AddTwoIntsRequest& req) -> AddTwoIntsResponse
            {
                println(
                    "[Server] Handling request_id=", req.request_id(),
                    "  a=", req.a(), "  b=", req.b()
                );
                AddTwoIntsResponse resp;
                resp.sum(req.a() + req.b());
                return resp;
            }
        );
        println("ServiceServer started for 'add_two_ints'");

        // ------------------------------------------------------------------ //
        // Service Clients
        // ------------------------------------------------------------------ //
        auto sync_client  = manager.createServiceClient<AddTwoIntsRequest, AddTwoIntsResponse>(
            "add_two_ints"
        );
        auto async_client = manager.createServiceClient<AddTwoIntsRequest, AddTwoIntsResponse>(
            "add_two_ints"
        );
        println("ServiceClients created");

        // Allow DDS discovery to complete before sending requests.
        println("Waiting for DDS discovery ...");
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // ------------------------------------------------------------------ //
        // Synchronous calls
        // ------------------------------------------------------------------ //
        println("\n--- Synchronous calls ---");

        for (int i = 1; i <= 3; ++i)
        {
            AddTwoIntsRequest req;
            req.a(i * 10);
            req.b(i * 3);

            println(
                "[SyncClient] Calling add_two_ints(", req.a(), ", ", req.b(), ") ..."
            );

            auto result = sync_client->call(req, std::chrono::milliseconds(3000));

            if (result.has_value())
            {
                println(
                    "[SyncClient] Response: sum=", result->sum(),
                    "  (expected ", req.a() + req.b(), ")"
                );
            }
            else
            {
                println("[SyncClient] ERROR: request timed out!");
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        // ------------------------------------------------------------------ //
        // Asynchronous calls
        // ------------------------------------------------------------------ //
        println("\n--- Asynchronous calls ---");

        std::atomic<int> async_done{0};
        const int async_count = 3;

        for (int i = 1; i <= async_count; ++i)
        {
            AddTwoIntsRequest req;
            req.a(i * 100);
            req.b(i * 7);

            println(
                "[AsyncClient] Sending add_two_ints(", req.a(), ", ", req.b(), ")"
            );

            async_client->callAsync(
                req,
                [i, expected = req.a() + req.b(), &async_done](const AddTwoIntsResponse& resp)
                {
                    println(
                        "[AsyncClient] Response #", i, ": sum=", resp.sum(),
                        "  (expected ", expected, ")"
                    );
                    async_done.fetch_add(1);
                }
            );
        }

        // Wait for all async responses (with a generous timeout).
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
        while (async_done.load() < async_count &&
               std::chrono::steady_clock::now() < deadline)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (async_done.load() < async_count)
        {
            println("[AsyncClient] WARNING: only ", async_done.load(),
                    "/", async_count, " async responses received");
        }

        // ------------------------------------------------------------------ //
        // Statistics
        // ------------------------------------------------------------------ //
        println("\n--- Statistics ---");
        println("Server handled:         ", server->getTotalHandled(),   " requests");
        println("SyncClient  successful: ", sync_client->getTotalCalls(), " calls");
        println("SyncClient  timeouts:   ", sync_client->getTimeoutCalls(), " calls");
        println("AsyncClient successful: ", async_client->getTotalCalls(), " calls");

        println("\nService demo completed successfully.");
    }
    catch (const DDSException& e)
    {
        std::cerr << "DDS Exception: " << e.getMessage() << '\n';
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
