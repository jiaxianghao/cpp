#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <chrono>
#include <functional>
#include "httplib.h"
#include "api_protocol.h"

// Callback function type for async responses
using ResponseCallback = std::function<void(const ApiResponse&)>;

class AsyncApiClient
{
private:
    httplib::Client client;
    std::string server_url;
    
public:
    AsyncApiClient(const std::string& url = "http://localhost:8080") 
        : client(url), server_url(url) {}
    
    // Async method with callback
    void executeCommandAsync(const std::string& command, 
                            const std::string& data, 
                            ResponseCallback callback)
    {
        // Launch async task
        std::thread([this, command, data, callback]()
        {
            try
            {
                std::cout << "[Async] Starting request - Command: " << command 
                         << ", Data: " << data << std::endl;
                
                // Prepare request parameters
                httplib::Params params;
                params.emplace("command", command);
                params.emplace("data", data);
                
                // Make HTTP POST request (this is still blocking within the thread)
                auto res = client.Post("/api/execute", params);
                
                ApiResponse response;
                
                if (!res)
                {
                    response.status_code = -1;
                    response.success = false;
                    response.message = "Failed to connect to server";
                    response.result = "";
                }
                else if (res->status != 200)
                {
                    response.status_code = res->status;
                    response.success = false;
                    response.message = "Server returned error status: " + std::to_string(res->status);
                    response.result = res->body;
                }
                else
                {
                    response.status_code = 200;
                    response.success = true;
                    response.message = "Request completed";
                    response.result = res->body;
                }
                
                std::cout << "[Async] Request completed, calling callback..." << std::endl;
                
                // Call the callback with the result
                callback(response);
            }
            catch (const std::exception& e)
            {
                ApiResponse error_response;
                error_response.status_code = -1;
                error_response.success = false;
                error_response.message = "Exception occurred: " + std::string(e.what());
                error_response.result = "";
                
                callback(error_response);
            }
        }).detach(); // Detach the thread so it runs independently
    }
    
    // Async method returning future
    std::future<ApiResponse> executeCommandFuture(const std::string& command, 
                                                  const std::string& data = "")
    {
        return std::async(std::launch::async, [this, command, data]() -> ApiResponse
        {
            try
            {
                std::cout << "[Future] Starting request - Command: " << command 
                         << ", Data: " << data << std::endl;
                
                httplib::Params params;
                params.emplace("command", command);
                params.emplace("data", data);
                
                auto res = client.Post("/api/execute", params);
                
                ApiResponse response;
                
                if (!res)
                {
                    response.status_code = -1;
                    response.success = false;
                    response.message = "Failed to connect to server";
                    response.result = "";
                }
                else if (res->status != 200)
                {
                    response.status_code = res->status;
                    response.success = false;
                    response.message = "Server returned error status: " + std::to_string(res->status);
                    response.result = res->body;
                }
                else
                {
                    response.status_code = 200;
                    response.success = true;
                    response.message = "Request completed";
                    response.result = res->body;
                }
                
                std::cout << "[Future] Request completed" << std::endl;
                return response;
            }
            catch (const std::exception& e)
            {
                ApiResponse error_response;
                error_response.status_code = -1;
                error_response.success = false;
                error_response.message = "Exception occurred: " + std::string(e.what());
                error_response.result = "";
                return error_response;
            }
        });
    }
    
    // Multiple async requests with callback
    void executeMultipleCommandsAsync(const std::vector<std::pair<std::string, std::string>>& commands,
                                     ResponseCallback callback)
    {
        std::cout << "[Multi-Async] Starting " << commands.size() << " requests..." << std::endl;
        
        for (const auto& cmd : commands)
        {
            executeCommandAsync(cmd.first, cmd.second, [callback, cmd](const ApiResponse& response)
            {
                std::cout << "[Multi-Async] Command '" << cmd.first << "' completed" << std::endl;
                callback(response);
            });
        }
    }
    
    bool checkServerHealth()
    {
        try
        {
            auto res = client.Get("/health");
            return res && res->status == 200;
        }
        catch (...)
        {
            return false;
        }
    }
};

void printUsage(const char* program_name)
{
    std::cout << "Usage: " << program_name << " <mode> <command> [data]" << std::endl;
    std::cout << "Modes:" << std::endl;
    std::cout << "  callback <command> [data]  - Async with callback" << std::endl;
    std::cout << "  future <command> [data]   - Async with future" << std::endl;
    std::cout << "  multi                     - Multiple async requests" << std::endl;
    std::cout << "  health                    - Check server health" << std::endl;
    std::cout << "\nAvailable commands:" << std::endl;
    std::cout << "  length <string>  - Get string length" << std::endl;
    std::cout << "  uppercase <string> - Convert to uppercase" << std::endl;
    std::cout << "  reverse <string> - Reverse the string" << std::endl;
}

void demoCallbackMode(const std::string& command, const std::string& data)
{
    AsyncApiClient client;
    
    std::cout << "\n=== Callback Mode Demo ===" << std::endl;
    std::cout << "Sending async request with callback..." << std::endl;
    
    // Send async request with callback
    client.executeCommandAsync(command, data, [](const ApiResponse& response)
    {
        std::cout << "\n=== Callback Response ===" << std::endl;
        std::cout << "Status Code: " << response.status_code << std::endl;
        std::cout << "Success: " << (response.success ? "Yes" : "No") << std::endl;
        std::cout << "Message: " << response.message << std::endl;
        std::cout << "Result: " << response.result << std::endl;
    });
    
    // Do other work while waiting for response
    std::cout << "Main thread continues working..." << std::endl;
    for (int i = 0; i < 5; ++i)
    {
        std::cout << "Working... " << (i + 1) << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    // Wait a bit for the async response
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

void demoFutureMode(const std::string& command, const std::string& data)
{
    AsyncApiClient client;
    
    std::cout << "\n=== Future Mode Demo ===" << std::endl;
    std::cout << "Sending async request with future..." << std::endl;
    
    // Send async request and get future
    auto future_response = client.executeCommandFuture(command, data);
    
    // Do other work while waiting
    std::cout << "Main thread continues working..." << std::endl;
    for (int i = 0; i < 3; ++i)
    {
        std::cout << "Working... " << (i + 1) << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    
    // Wait for the result (this will block until ready)
    std::cout << "Waiting for async result..." << std::endl;
    ApiResponse response = future_response.get();
    
    std::cout << "\n=== Future Response ===" << std::endl;
    std::cout << "Status Code: " << response.status_code << std::endl;
    std::cout << "Success: " << (response.success ? "Yes" : "No") << std::endl;
    std::cout << "Message: " << response.message << std::endl;
    std::cout << "Result: " << response.result << std::endl;
}

void demoMultiMode()
{
    AsyncApiClient client;
    
    std::cout << "\n=== Multi-Async Mode Demo ===" << std::endl;
    
    // Prepare multiple commands
    std::vector<std::pair<std::string, std::string>> commands = {
        {"length", "Hello World"},
        {"uppercase", "hello world"},
        {"reverse", "Hello World"}
    };
    
    std::cout << "Sending " << commands.size() << " async requests..." << std::endl;
    
    int completed_count = 0;
    
    // Send multiple async requests
    client.executeMultipleCommandsAsync(commands, [&completed_count](const ApiResponse& response)
    {
        completed_count++;
        std::cout << "\n=== Multi-Async Response #" << completed_count << " ===" << std::endl;
        std::cout << "Status Code: " << response.status_code << std::endl;
        std::cout << "Success: " << (response.success ? "Yes" : "No") << std::endl;
        std::cout << "Message: " << response.message << std::endl;
        std::cout << "Result: " << response.result << std::endl;
    });
    
    // Do other work while waiting
    std::cout << "Main thread continues working..." << std::endl;
    for (int i = 0; i < 5; ++i)
    {
        std::cout << "Working... " << (i + 1) << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
    
    // Wait for all responses
    std::cout << "Waiting for all async responses..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string mode = argv[1];
    
    AsyncApiClient client;
    
    // Check server health first
    if (!client.checkServerHealth())
    {
        std::cerr << "Error: Cannot connect to server. Make sure the server is running on localhost:8080" << std::endl;
        return 1;
    }
    
    if (mode == "health")
    {
        std::cout << "Server is healthy and responding" << std::endl;
        return 0;
    }
    else if (mode == "callback")
    {
        if (argc < 3)
        {
            std::cerr << "Error: Command required for callback mode" << std::endl;
            return 1;
        }
        std::string command = argv[2];
        std::string data = (argc > 3) ? argv[3] : "";
        demoCallbackMode(command, data);
    }
    else if (mode == "future")
    {
        if (argc < 3)
        {
            std::cerr << "Error: Command required for future mode" << std::endl;
            return 1;
        }
        std::string command = argv[2];
        std::string data = (argc > 3) ? argv[3] : "";
        demoFutureMode(command, data);
    }
    else if (mode == "multi")
    {
        demoMultiMode();
    }
    else
    {
        std::cerr << "Error: Unknown mode '" << mode << "'" << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    return 0;
}
