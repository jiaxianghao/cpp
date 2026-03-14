#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "httplib.h"
#include "api_protocol.h"

class ApiServer
{
private:
    httplib::Server server;
    int port;
    
public:
    ApiServer(int p = 8080) : port(p) {}
    
    // Business logic - example string processing
    std::string processString(const std::string& input)
    {
        // Simulate some processing time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Example: reverse the string
        std::string result = input;
        std::reverse(result.begin(), result.end());
        return result;
    }
    
    // Business logic - example calculation
    std::string calculate(const std::string& operation, const std::string& data)
    {
        if (operation == "length")
        {
            return std::to_string(data.length());
        }
        else if (operation == "uppercase")
        {
            std::string result = data;
            std::transform(result.begin(), result.end(), result.begin(), ::toupper);
            return result;
        }
        else if (operation == "reverse")
        {
            return processString(data);
        }
        else
        {
            return "Unknown operation: " + operation;
        }
    }
    
    void setupRoutes()
    {
        // Health check endpoint
        server.Get("/health", [](const httplib::Request&, httplib::Response& res)
        {
            res.set_content("OK", "text/plain");
        });
        
        // Main API endpoint
        server.Post("/api/execute", [this](const httplib::Request& req, httplib::Response& res)
        {
            try
            {
                // Parse request data
                std::string command = req.get_param_value("command");
                std::string data = req.get_param_value("data");
                
                if (command.empty())
                {
                    res.status = 400;
                    res.set_content(SimpleJsonBuilder::buildErrorResponse(400, "Missing command parameter"), "application/json");
                    return;
                }
                
                std::cout << "Received request - Command: " << command << ", Data: " << data << std::endl;
                
                // Execute business logic
                std::string result = calculate(command, data);
                
                // Build response
                ApiResponse response;
                response.status_code = 200;
                response.success = true;
                response.message = "Operation completed successfully";
                response.result = result;
                
                res.status = 200;
                res.set_content(SimpleJsonBuilder::buildResponse(response), "application/json");
                
                std::cout << "Response sent - Result: " << result << std::endl;
            }
            catch (const std::exception& e)
            {
                res.status = 500;
                res.set_content(SimpleJsonBuilder::buildErrorResponse(500, "Internal server error: " + std::string(e.what())), "application/json");
            }
        });
    }
    
    bool start()
    {
        setupRoutes();
        
        std::cout << "Starting API server on port " << port << std::endl;
        std::cout << "Available endpoints:" << std::endl;
        std::cout << "  GET  /health" << std::endl;
        std::cout << "  POST /api/execute" << std::endl;
        std::cout << "Server is running... Press Ctrl+C to stop" << std::endl;
        
        return server.listen("0.0.0.0", port);
    }
};

int main()
{
    ApiServer server(8080);
    
    if (!server.start())
    {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    return 0;
}
