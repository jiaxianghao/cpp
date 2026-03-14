#include <iostream>
#include <string>
#include "httplib.h"
#include "api_protocol.h"

class ApiClient
{
private:
    httplib::Client client;
    std::string server_url;
    
public:
    ApiClient(const std::string& url = "http://localhost:8080") 
        : client(url), server_url(url) {}
    
    ApiResponse executeCommand(const std::string& command, const std::string& data = "")
    {
        ApiResponse response;
        
        try
        {
            // Prepare request parameters
            httplib::Params params;
            params.emplace("command", command);
            params.emplace("data", data);
            
            std::cout << "Sending request to " << server_url << "/api/execute" << std::endl;
            std::cout << "Command: " << command << ", Data: " << data << std::endl;
            
            // Make HTTP POST request
            auto res = client.Post("/api/execute", params);
            
            if (!res)
            {
                response.status_code = -1;
                response.success = false;
                response.message = "Failed to connect to server";
                response.result = "";
                return response;
            }
            
            if (res->status != 200)
            {
                response.status_code = res->status;
                response.success = false;
                response.message = "Server returned error status: " + std::to_string(res->status);
                response.result = res->body;
                return response;
            }
            
            // Parse response (simplified - in real implementation you'd use a JSON parser)
            response.status_code = 200;
            response.success = true;
            response.message = "Request completed";
            response.result = res->body;
            
            std::cout << "Response received: " << res->body << std::endl;
        }
        catch (const std::exception& e)
        {
            response.status_code = -1;
            response.success = false;
            response.message = "Exception occurred: " + std::string(e.what());
            response.result = "";
        }
        
        return response;
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
    std::cout << "Usage: " << program_name << " <command> [data]" << std::endl;
    std::cout << "Available commands:" << std::endl;
    std::cout << "  length <string>  - Get string length" << std::endl;
    std::cout << "  uppercase <string> - Convert to uppercase" << std::endl;
    std::cout << "  reverse <string> - Reverse the string" << std::endl;
    std::cout << "  health - Check server health" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string command = argv[1];
    std::string data = (argc > 2) ? argv[2] : "";
    
    ApiClient client;
    
    // Check server health first
    if (!client.checkServerHealth())
    {
        std::cerr << "Error: Cannot connect to server. Make sure the server is running on localhost:8080" << std::endl;
        return 1;
    }
    
    if (command == "health")
    {
        std::cout << "Server is healthy and responding" << std::endl;
        return 0;
    }
    
    // Execute the command
    ApiResponse response = client.executeCommand(command, data);
    
    // Print results
    std::cout << "\n=== Execution Results ===" << std::endl;
    std::cout << "Status Code: " << response.status_code << std::endl;
    std::cout << "Success: " << (response.success ? "Yes" : "No") << std::endl;
    std::cout << "Message: " << response.message << std::endl;
    std::cout << "Result: " << response.result << std::endl;
    
    return response.success ? 0 : 1;
}
