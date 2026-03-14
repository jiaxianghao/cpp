#pragma once

#include <string>
#include <map>

// Request structure for API calls
struct ApiRequest
{
    std::string command;
    std::map<std::string, std::string> parameters;
    std::string data;
};

// Response structure for API calls
struct ApiResponse
{
    int status_code;
    std::string message;
    std::string result;
    bool success;
};

// Simple JSON-like string builder for responses
class SimpleJsonBuilder
{
public:
    static std::string buildResponse(const ApiResponse& response)
    {
        std::string json = "{";
        json += "\"status_code\":" + std::to_string(response.status_code) + ",";
        json += "\"success\":" + std::string(response.success ? "true" : "false") + ",";
        json += "\"message\":\"" + response.message + "\",";
        json += "\"result\":\"" + response.result + "\"";
        json += "}";
        return json;
    }
    
    static std::string buildErrorResponse(int status_code, const std::string& message)
    {
        ApiResponse error_response;
        error_response.status_code = status_code;
        error_response.success = false;
        error_response.message = message;
        error_response.result = "";
        return buildResponse(error_response);
    }
};
