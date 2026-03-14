#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <sstream>
#include "database/database_config.h"
#include "database/database_connection.h"
#include "repositories/user_repository.h"
#include "services/user_service.h"
#include "controllers/user_controller.h"

/**
 * Web API 示例 - 展示如何构建RESTful API接口
 * 这个示例模拟了Web API的使用场景
 */

// 模拟HTTP请求结构
struct HttpRequest
{
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> query_params;
    std::string body;
};

// 模拟HTTP响应结构
struct HttpResponse
{
    int status_code;
    std::map<std::string, std::string> headers;
    std::string body;
};

class WebAPI
{
private:
    std::shared_ptr<UserController> user_controller_;
    
public:
    WebAPI()
    {
        initializeDatabase();
        setupUserController();
    }
    
    void initializeDatabase()
    {
        DatabaseConfig& config = DatabaseConfig::getInstance();
        config.setHost("localhost");
        config.setUser("root");
        config.setPassword("password");
        config.setDatabase("myapp");
        config.setPort(3306);
    }
    
    void setupUserController()
    {
        auto connection = std::make_shared<DatabaseConnection>();
        if (!connection->connect())
        {
            throw std::runtime_error("数据库连接失败: " + connection->getLastError());
        }
        
        auto user_repo = std::make_shared<UserRepository>(connection);
        auto user_service = std::make_shared<UserService>(user_repo);
        user_controller_ = std::make_shared<UserController>(user_service);
    }
    
    /**
     * 处理HTTP请求
     */
    HttpResponse handleRequest(const HttpRequest& request)
    {
        HttpResponse response;
        response.headers["Content-Type"] = "application/json";
        
        try
        {
            if (request.path == "/api/users" && request.method == "GET")
            {
                return handleGetUsers(request);
            }
            else if (request.path == "/api/users" && request.method == "POST")
            {
                return handleCreateUser(request);
            }
            else if (request.path.find("/api/users/") == 0 && request.method == "GET")
            {
                return handleGetUser(request);
            }
            else if (request.path.find("/api/users/") == 0 && request.method == "PUT")
            {
                return handleUpdateUser(request);
            }
            else if (request.path.find("/api/users/") == 0 && request.method == "DELETE")
            {
                return handleDeleteUser(request);
            }
            else if (request.path == "/api/users/search" && request.method == "GET")
            {
                return handleSearchUsers(request);
            }
            else if (request.path == "/api/users/stats" && request.method == "GET")
            {
                return handleGetStats(request);
            }
            else
            {
                response.status_code = 404;
                response.body = "{\"error\": \"Not Found\"}";
            }
        }
        catch (const std::exception& e)
        {
            response.status_code = 500;
            response.body = "{\"error\": \"" + std::string(e.what()) + "\"}";
        }
        
        return response;
    }
    
    /**
     * GET /api/users - 获取所有用户
     */
    HttpResponse handleGetUsers(const HttpRequest& request)
    {
        HttpResponse response;
        response.status_code = 200;
        
        auto users = user_controller_->getAllUsers();
        
        std::ostringstream json;
        json << "{\"users\":[";
        for (size_t i = 0; i < users.size(); ++i)
        {
            if (i > 0) json << ",";
            json << "{\"id\":" << users[i].getId()
                 << ",\"username\":\"" << users[i].getUsername() << "\""
                 << ",\"email\":\"" << users[i].getEmail() << "\""
                 << ",\"created_at\":\"" << users[i].getCreatedAt() << "\"}";
        }
        json << "]}";
        
        response.body = json.str();
        return response;
    }
    
    /**
     * POST /api/users - 创建用户
     */
    HttpResponse handleCreateUser(const HttpRequest& request)
    {
        HttpResponse response;
        
        // 解析JSON请求体 (简化版本)
        std::string username, email;
        parseCreateUserRequest(request.body, username, email);
        
        if (username.empty() || email.empty())
        {
            response.status_code = 400;
            response.body = "{\"error\": \"Username and email are required\"}";
            return response;
        }
        
        auto result = user_controller_->createUser(username, email);
        
        if (result.success)
        {
            response.status_code = 201;
            std::ostringstream json;
            json << "{\"id\":" << result.user.getId()
                 << ",\"username\":\"" << result.user.getUsername() << "\""
                 << ",\"email\":\"" << result.user.getEmail() << "\""
                 << ",\"created_at\":\"" << result.user.getCreatedAt() << "\"}";
            response.body = json.str();
        }
        else
        {
            response.status_code = 400;
            response.body = "{\"error\":\"" + result.error_message + "\"}";
        }
        
        return response;
    }
    
    /**
     * GET /api/users/{id} - 获取特定用户
     */
    HttpResponse handleGetUser(const HttpRequest& request)
    {
        HttpResponse response;
        
        // 从路径中提取用户ID
        int user_id = extractUserIdFromPath(request.path);
        if (user_id <= 0)
        {
            response.status_code = 400;
            response.body = "{\"error\": \"Invalid user ID\"}";
            return response;
        }
        
        User user = user_controller_->getUserById(user_id);
        if (user.getId() > 0)
        {
            response.status_code = 200;
            std::ostringstream json;
            json << "{\"id\":" << user.getId()
                 << ",\"username\":\"" << user.getUsername() << "\""
                 << ",\"email\":\"" << user.getEmail() << "\""
                 << ",\"created_at\":\"" << user.getCreatedAt() << "\"}";
            response.body = json.str();
        }
        else
        {
            response.status_code = 404;
            response.body = "{\"error\": \"User not found\"}";
        }
        
        return response;
    }
    
    /**
     * PUT /api/users/{id} - 更新用户
     */
    HttpResponse handleUpdateUser(const HttpRequest& request)
    {
        HttpResponse response;
        
        int user_id = extractUserIdFromPath(request.path);
        if (user_id <= 0)
        {
            response.status_code = 400;
            response.body = "{\"error\": \"Invalid user ID\"}";
            return response;
        }
        
        std::string new_email;
        parseUpdateUserRequest(request.body, new_email);
        
        if (new_email.empty())
        {
            response.status_code = 400;
            response.body = "{\"error\": \"Email is required\"}";
            return response;
        }
        
        User user = user_controller_->getUserById(user_id);
        if (user.getId() <= 0)
        {
            response.status_code = 404;
            response.body = "{\"error\": \"User not found\"}";
            return response;
        }
        
        user.setEmail(new_email);
        auto result = user_controller_->updateUser(user);
        
        if (result.success)
        {
            response.status_code = 200;
            std::ostringstream json;
            json << "{\"id\":" << user.getId()
                 << ",\"username\":\"" << user.getUsername() << "\""
                 << ",\"email\":\"" << user.getEmail() << "\""
                 << ",\"created_at\":\"" << user.getCreatedAt() << "\"}";
            response.body = json.str();
        }
        else
        {
            response.status_code = 400;
            response.body = "{\"error\":\"" + result.error_message + "\"}";
        }
        
        return response;
    }
    
    /**
     * DELETE /api/users/{id} - 删除用户
     */
    HttpResponse handleDeleteUser(const HttpRequest& request)
    {
        HttpResponse response;
        
        int user_id = extractUserIdFromPath(request.path);
        if (user_id <= 0)
        {
            response.status_code = 400;
            response.body = "{\"error\": \"Invalid user ID\"}";
            return response;
        }
        
        auto result = user_controller_->deleteUser(user_id);
        
        if (result.success)
        {
            response.status_code = 204;
            response.body = "";
        }
        else
        {
            response.status_code = 400;
            response.body = "{\"error\":\"" + result.error_message + "\"}";
        }
        
        return response;
    }
    
    /**
     * GET /api/users/search?q=keyword - 搜索用户
     */
    HttpResponse handleSearchUsers(const HttpRequest& request)
    {
        HttpResponse response;
        response.status_code = 200;
        
        auto it = request.query_params.find("q");
        if (it == request.query_params.end())
        {
            response.status_code = 400;
            response.body = "{\"error\": \"Search query parameter 'q' is required\"}";
            return response;
        }
        
        std::string keyword = it->second;
        auto users = user_controller_->searchUsers(keyword);
        
        std::ostringstream json;
        json << "{\"users\":[";
        for (size_t i = 0; i < users.size(); ++i)
        {
            if (i > 0) json << ",";
            json << "{\"id\":" << users[i].getId()
                 << ",\"username\":\"" << users[i].getUsername() << "\""
                 << ",\"email\":\"" << users[i].getEmail() << "\""
                 << ",\"created_at\":\"" << users[i].getCreatedAt() << "\"}";
        }
        json << "]}";
        
        response.body = json.str();
        return response;
    }
    
    /**
     * GET /api/users/stats - 获取用户统计信息
     */
    HttpResponse handleGetStats(const HttpRequest& request)
    {
        HttpResponse response;
        response.status_code = 200;
        
        std::string stats = user_controller_->getUserStatistics();
        response.body = "{\"statistics\":\"" + stats + "\"}";
        
        return response;
    }
    
private:
    void parseCreateUserRequest(const std::string& body, std::string& username, std::string& email)
    {
        // 简化的JSON解析 (实际项目中应使用JSON库)
        size_t username_pos = body.find("\"username\":\"");
        size_t email_pos = body.find("\"email\":\"");
        
        if (username_pos != std::string::npos)
        {
            username_pos += 11; // "username":"
            size_t end_pos = body.find("\"", username_pos);
            username = body.substr(username_pos, end_pos - username_pos);
        }
        
        if (email_pos != std::string::npos)
        {
            email_pos += 8; // "email":"
            size_t end_pos = body.find("\"", email_pos);
            email = body.substr(email_pos, end_pos - email_pos);
        }
    }
    
    void parseUpdateUserRequest(const std::string& body, std::string& email)
    {
        size_t email_pos = body.find("\"email\":\"");
        if (email_pos != std::string::npos)
        {
            email_pos += 8; // "email":"
            size_t end_pos = body.find("\"", email_pos);
            email = body.substr(email_pos, end_pos - email_pos);
        }
    }
    
    int extractUserIdFromPath(const std::string& path)
    {
        // 从 "/api/users/123" 中提取 123
        size_t last_slash = path.find_last_of('/');
        if (last_slash != std::string::npos)
        {
            std::string id_str = path.substr(last_slash + 1);
            return std::stoi(id_str);
        }
        return -1;
    }
};

/**
 * 模拟HTTP请求
 */
void simulateWebAPIRequests()
{
    WebAPI api;
    
    std::cout << "🌐 Web API 示例演示" << std::endl;
    
    // 模拟请求1: 创建用户
    std::cout << "\n1. POST /api/users - 创建用户" << std::endl;
    HttpRequest create_req;
    create_req.method = "POST";
    create_req.path = "/api/users";
    create_req.body = "{\"username\":\"api_user\",\"email\":\"api@example.com\"}";
    
    HttpResponse create_resp = api.handleRequest(create_req);
    std::cout << "状态码: " << create_resp.status_code << std::endl;
    std::cout << "响应: " << create_resp.body << std::endl;
    
    // 模拟请求2: 获取所有用户
    std::cout << "\n2. GET /api/users - 获取所有用户" << std::endl;
    HttpRequest get_all_req;
    get_all_req.method = "GET";
    get_all_req.path = "/api/users";
    
    HttpResponse get_all_resp = api.handleRequest(get_all_req);
    std::cout << "状态码: " << get_all_resp.status_code << std::endl;
    std::cout << "响应: " << get_all_resp.body << std::endl;
    
    // 模拟请求3: 获取特定用户
    std::cout << "\n3. GET /api/users/1 - 获取用户ID=1" << std::endl;
    HttpRequest get_user_req;
    get_user_req.method = "GET";
    get_user_req.path = "/api/users/1";
    
    HttpResponse get_user_resp = api.handleRequest(get_user_req);
    std::cout << "状态码: " << get_user_resp.status_code << std::endl;
    std::cout << "响应: " << get_user_resp.body << std::endl;
    
    // 模拟请求4: 搜索用户
    std::cout << "\n4. GET /api/users/search?q=api - 搜索用户" << std::endl;
    HttpRequest search_req;
    search_req.method = "GET";
    search_req.path = "/api/users/search";
    search_req.query_params["q"] = "api";
    
    HttpResponse search_resp = api.handleRequest(search_req);
    std::cout << "状态码: " << search_resp.status_code << std::endl;
    std::cout << "响应: " << search_resp.body << std::endl;
    
    // 模拟请求5: 获取统计信息
    std::cout << "\n5. GET /api/users/stats - 获取统计信息" << std::endl;
    HttpRequest stats_req;
    stats_req.method = "GET";
    stats_req.path = "/api/users/stats";
    
    HttpResponse stats_resp = api.handleRequest(stats_req);
    std::cout << "状态码: " << stats_resp.status_code << std::endl;
    std::cout << "响应: " << stats_resp.body << std::endl;
}

int main()
{
    std::cout << "=== Web API 示例 ===" << std::endl;
    std::cout << "这个示例展示了如何构建RESTful API接口" << std::endl;
    
    simulateWebAPIRequests();
    
    return 0;
}
