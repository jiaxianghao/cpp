#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

// HTTP Request structure
struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    std::unordered_map<std::string, std::string> query_params;
    std::unordered_map<std::string, std::string> path_params;
    
    // Helper methods
    std::string getHeader(const std::string& name) const;
    std::string getQueryParam(const std::string& name) const;
    std::string getPathParam(const std::string& name) const;
    bool hasHeader(const std::string& name) const;
    bool hasQueryParam(const std::string& name) const;
    bool hasPathParam(const std::string& name) const;
};

// HTTP Response structure
struct HttpResponse {
    int status_code = 200;
    std::string status_message = "OK";
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    
    // Helper methods
    void setStatus(int code, const std::string& message = "");
    void setHeader(const std::string& name, const std::string& value);
    void setContentType(const std::string& content_type);
    void setBody(const std::string& body_content);
    void setJsonBody(const std::string& json_content);
    void setError(int code, const std::string& message);
    
    // Common responses
    static HttpResponse ok(const std::string& body = "");
    static HttpResponse json(const std::string& json_data);
    static HttpResponse error(int code, const std::string& message);
    static HttpResponse notFound(const std::string& message = "Not Found");
    static HttpResponse badRequest(const std::string& message = "Bad Request");
    static HttpResponse internalServerError(const std::string& message = "Internal Server Error");
};

// Route handler function type
using RouteHandler = std::function<HttpResponse(const HttpRequest&)>;

// Route structure
struct Route {
    std::string method;
    std::string path;
    RouteHandler handler;
    std::string name;
    std::string description;
};

// HTTP Server class
class HttpServer {
public:
    HttpServer(const std::string& host = "0.0.0.0", int port = 8080);
    ~HttpServer();
    
    // Delete copy constructor and assignment operator
    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;
    
    // Route registration
    void get(const std::string& path, RouteHandler handler, const std::string& name = "", const std::string& description = "");
    void post(const std::string& path, RouteHandler handler, const std::string& name = "", const std::string& description = "");
    void put(const std::string& path, RouteHandler handler, const std::string& name = "", const std::string& description = "");
    void del(const std::string& path, RouteHandler handler, const std::string& name = "", const std::string& description = "");
    void patch(const std::string& path, RouteHandler handler, const std::string& name = "", const std::string& description = "");
    
    // Middleware support
    void use(RouteHandler middleware);
    
    // Server management
    bool start();
    void stop();
    bool isRunning() const;
    
    // Configuration
    void setHost(const std::string& host);
    void setPort(int port);
    void setThreadPoolSize(size_t size);
    void setRequestTimeout(int seconds);
    void enableCORS(const std::string& allowed_origins = "*");
    void enableSSL(const std::string& cert_file, const std::string& key_file);
    
    // Utility methods
    std::string getBaseUrl() const;
    std::vector<Route> getRoutes() const;
    void addStaticRoute(const std::string& url_path, const std::string& filesystem_path);
    
    // Error handling
    void setErrorHandler(RouteHandler handler);
    void setNotFoundHandler(RouteHandler handler);
    
private:
#ifdef _WIN32
    using SocketHandle = SOCKET;
    static constexpr SocketHandle kInvalidSocket = INVALID_SOCKET;
#else
    using SocketHandle = int;
    static constexpr SocketHandle kInvalidSocket = -1;
#endif

    std::string host_;
    int port_;
    size_t thread_pool_size_;
    int request_timeout_;
    bool enable_cors_;
    std::string cors_origins_;
    bool enable_ssl_;
    std::string ssl_cert_file_;
    std::string ssl_key_file_;
    
    std::vector<Route> routes_;
    std::vector<RouteHandler> middlewares_;
    RouteHandler error_handler_;
    RouteHandler not_found_handler_;
    
    std::atomic<bool> running_;
    std::unique_ptr<std::thread> server_thread_;
    SocketHandle server_socket_;
    
    // Route matching and parameter extraction
    bool matchRoute(const std::string& route_path, const std::string& request_path, 
                   std::unordered_map<std::string, std::string>& path_params);
    
    // Request processing
    HttpResponse processRequest(const HttpRequest& request);
    HttpResponse executeMiddleware(const HttpRequest& request, const RouteHandler& final_handler);
    
    // HTTP parsing and formatting
    HttpRequest parseHttpRequest(const std::string& raw_request);
    std::string formatHttpResponse(const HttpResponse& response);
    
    // Server loop
    void serverLoop();
    void handleClient(SocketHandle client_socket);

    // Socket helpers
    bool initializeServerSocket();
    void shutdownServerSocket();
    void closeSocket(SocketHandle socket_handle);
    
    // Utility functions
    std::vector<std::string> splitPath(const std::string& path) const;
    std::string urlDecode(const std::string& str) const;
    std::string urlEncode(const std::string& str) const;
    std::unordered_map<std::string, std::string> parseQueryString(const std::string& query) const;
    std::string getMimeType(const std::string& file_path) const;
    
    // Static file serving
    HttpResponse serveStaticFile(const std::string& file_path) const;
    bool isStaticFileRequest(const std::string& path) const;
    
    // CORS support
    void addCORSHeaders(HttpResponse& response) const;
};

// Utility functions for JSON handling
namespace JsonUtils {
    std::string jsonSuccess(const std::string& message = "Success");
    std::string jsonError(const std::string& message);
    std::string jsonData(const std::string& key, const std::string& value);
    std::string jsonObject(const std::unordered_map<std::string, std::string>& data);
    std::string jsonArray(const std::vector<std::string>& items);
    
    // Parse JSON (simple implementation)
    std::unordered_map<std::string, std::string> parseJson(const std::string& json_str);
    std::string getJsonValue(const std::string& json_str, const std::string& key);
}

#endif // HTTP_SERVER_H