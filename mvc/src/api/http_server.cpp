#include "api/http_server.h"
#include "utils/logger.h"
#include <sstream>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <regex>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <cctype>

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif

// HttpRequest implementation
std::string HttpRequest::getHeader(const std::string& name) const {
    auto it = headers.find(name);
    return (it != headers.end()) ? it->second : "";
}

std::string HttpRequest::getQueryParam(const std::string& name) const {
    auto it = query_params.find(name);
    return (it != query_params.end()) ? it->second : "";
}

std::string HttpRequest::getPathParam(const std::string& name) const {
    auto it = path_params.find(name);
    return (it != path_params.end()) ? it->second : "";
}

bool HttpRequest::hasHeader(const std::string& name) const {
    return headers.find(name) != headers.end();
}

bool HttpRequest::hasQueryParam(const std::string& name) const {
    return query_params.find(name) != query_params.end();
}

bool HttpRequest::hasPathParam(const std::string& name) const {
    return path_params.find(name) != path_params.end();
}

// HttpResponse implementation
void HttpResponse::setStatus(int code, const std::string& message) {
    status_code = code;
    status_message = message.empty() ? getDefaultStatusMessage(code) : message;
}

void HttpResponse::setHeader(const std::string& name, const std::string& value) {
    headers[name] = value;
}

void HttpResponse::setContentType(const std::string& content_type) {
    headers["Content-Type"] = content_type;
}

void HttpResponse::setBody(const std::string& body_content) {
    body = body_content;
    headers["Content-Length"] = std::to_string(body.length());
}

void HttpResponse::setJsonBody(const std::string& json_content) {
    setContentType("application/json");
    setBody(json_content);
}

void HttpResponse::setError(int code, const std::string& message) {
    setStatus(code, message);
    setJsonBody(JsonUtils::jsonError(message));
}

HttpResponse HttpResponse::ok(const std::string& body) {
    HttpResponse response;
    response.setStatus(200);
    response.setBody(body);
    return response;
}

HttpResponse HttpResponse::json(const std::string& json_data) {
    HttpResponse response;
    response.setStatus(200);
    response.setJsonBody(json_data);
    return response;
}

HttpResponse HttpResponse::error(int code, const std::string& message) {
    HttpResponse response;
    response.setError(code, message);
    return response;
}

HttpResponse HttpResponse::notFound(const std::string& message) {
    return error(404, message);
}

HttpResponse HttpResponse::badRequest(const std::string& message) {
    return error(400, message);
}

HttpResponse HttpResponse::internalServerError(const std::string& message) {
    return error(500, message);
}

std::string HttpResponse::getDefaultStatusMessage(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        case 503: return "Service Unavailable";
        default: return "Unknown";
    }
}

// HttpServer implementation
HttpServer::HttpServer(const std::string& host, int port)
    : host_(host), port_(port), thread_pool_size_(4), request_timeout_(30),
      enable_cors_(false), enable_ssl_(false), running_(false), server_socket_(kInvalidSocket) {
}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::get(const std::string& path, RouteHandler handler, const std::string& name, const std::string& description) {
    routes_.push_back({"GET", path, handler, name, description});
}

void HttpServer::post(const std::string& path, RouteHandler handler, const std::string& name, const std::string& description) {
    routes_.push_back({"POST", path, handler, name, description});
}

void HttpServer::put(const std::string& path, RouteHandler handler, const std::string& name, const std::string& description) {
    routes_.push_back({"PUT", path, handler, name, description});
}

void HttpServer::del(const std::string& path, RouteHandler handler, const std::string& name, const std::string& description) {
    routes_.push_back({"DELETE", path, handler, name, description});
}

void HttpServer::patch(const std::string& path, RouteHandler handler, const std::string& name, const std::string& description) {
    routes_.push_back({"PATCH", path, handler, name, description});
}

void HttpServer::use(RouteHandler middleware) {
    middlewares_.push_back(middleware);
}

bool HttpServer::start() {
    if (running_) {
        Logger::warn("HTTP server is already running");
        return false;
    }

    Logger::info("Starting HTTP server on {}:{}", host_, port_);
    
    try {
        running_ = true;
        server_thread_ = std::make_unique<std::thread>(&HttpServer::serverLoop, this);
        
        Logger::info("HTTP server started successfully");
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to start HTTP server: {}", e.what());
        running_ = false;
        return false;
    }
}

void HttpServer::stop() {
    if (!running_) {
        return;
    }

    Logger::info("Stopping HTTP server...");
    running_ = false;
    shutdownServerSocket();

    if (server_thread_ && server_thread_->joinable()) {
        server_thread_->join();
    }

    Logger::info("HTTP server stopped");
}

bool HttpServer::isRunning() const {
    return running_;
}

void HttpServer::setHost(const std::string& host) {
    host_ = host;
}

void HttpServer::setPort(int port) {
    port_ = port;
}

void HttpServer::setThreadPoolSize(size_t size) {
    thread_pool_size_ = size;
}

void HttpServer::setRequestTimeout(int seconds) {
    request_timeout_ = seconds;
}

void HttpServer::enableCORS(const std::string& allowed_origins) {
    enable_cors_ = true;
    cors_origins_ = allowed_origins;
}

void HttpServer::enableSSL(const std::string& cert_file, const std::string& key_file) {
    enable_ssl_ = true;
    ssl_cert_file_ = cert_file;
    ssl_key_file_ = key_file;
}

std::string HttpServer::getBaseUrl() const {
    std::string protocol = enable_ssl_ ? "https://" : "http://";
    return protocol + host_ + ":" + std::to_string(port_);
}

std::vector<Route> HttpServer::getRoutes() const {
    return routes_;
}

void HttpServer::addStaticRoute(const std::string& url_path, const std::string& filesystem_path) {
    // Add a route for static files
    get(url_path + "/*", [this, filesystem_path](const HttpRequest& req) {
        std::string file_path = req.path.substr(url_path.length());
        if (file_path.empty() || file_path == "/") {
            file_path = "/index.html";
        }
        
        std::string full_path = filesystem_path + file_path;
        return serveStaticFile(full_path);
    }, "static_files", "Serve static files");
}

void HttpServer::setErrorHandler(RouteHandler handler) {
    error_handler_ = handler;
}

void HttpServer::setNotFoundHandler(RouteHandler handler) {
    not_found_handler_ = handler;
}

bool HttpServer::matchRoute(const std::string& route_path, const std::string& request_path, 
                           std::unordered_map<std::string, std::string>& path_params) {
    // Simple route matching with parameter extraction
    // This is a basic implementation - in production, use a proper routing library
    
    std::vector<std::string> route_parts = splitPath(route_path);
    std::vector<std::string> request_parts = splitPath(request_path);
    
    if (route_parts.size() != request_parts.size()) {
        return false;
    }
    
    path_params.clear();
    
    for (size_t i = 0; i < route_parts.size(); ++i) {
        if (route_parts[i].empty() || request_parts[i].empty()) {
            continue;
        }
        
        if (route_parts[i][0] == ':' && route_parts[i].length() > 1) {
            // Parameter
            std::string param_name = route_parts[i].substr(1);
            path_params[param_name] = urlDecode(request_parts[i]);
        } else if (route_parts[i] != request_parts[i]) {
            return false;
        }
    }
    
    return true;
}

HttpResponse HttpServer::processRequest(const HttpRequest& request) {
    Logger::debug("Processing {} request to {}", request.method, request.path);
    
    // Find matching route
    for (const auto& route : routes_) {
        if (route.method == request.method) {
            std::unordered_map<std::string, std::string> path_params;
            if (matchRoute(route.path, request.path, path_params)) {
                // Create a copy of the request with path parameters
                HttpRequest modified_request = request;
                modified_request.path_params = path_params;
                
                // Execute middleware and route handler
                return executeMiddleware(modified_request, route.handler);
            }
        }
    }
    
    // No matching route found
    if (not_found_handler_) {
        return not_found_handler_(request);
    }
    
    return HttpResponse::notFound("Route not found: " + request.method + " " + request.path);
}

HttpResponse HttpServer::executeMiddleware(const HttpRequest& request, const RouteHandler& final_handler) {
    // Execute middleware chain
    HttpResponse response;
    
    try {
        // Apply CORS if enabled
        if (enable_cors_) {
            response.setHeader("Access-Control-Allow-Origin", cors_origins_);
            response.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, PATCH, OPTIONS");
            response.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        }
        
        // Execute middleware
        for (const auto& middleware : middlewares_) {
            response = middleware(request);
            if (response.status_code != 200) {
                return response; // Middleware stopped the request
            }
        }
        
        // Execute final handler
        response = final_handler(request);
        
    } catch (const std::exception& e) {
        Logger::error("Error processing request: {}", e.what());
        response = HttpResponse::internalServerError(e.what());
    }
    
    return response;
}

HttpRequest HttpServer::parseHttpRequest(const std::string& raw_request) {
    HttpRequest request;
    std::istringstream stream(raw_request);
    std::string line;
    
    // Parse request line
    if (std::getline(stream, line)) {
        std::istringstream line_stream(line);
        line_stream >> request.method >> request.path >> request.version;
        
        // Parse query parameters
        size_t query_pos = request.path.find('?');
        if (query_pos != std::string::npos) {
            std::string query_string = request.path.substr(query_pos + 1);
            request.path = request.path.substr(0, query_pos);
            request.query_params = parseQueryString(query_string);
        }
    }
    
    // Parse headers
    while (std::getline(stream, line) && line != "\r") {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string name = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            
            // Trim whitespace
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            
            request.headers[name] = value;
        }
    }
    
    // Parse body
    std::string body_line;
    while (std::getline(stream, body_line)) {
        request.body += body_line + "\n";
    }
    
    // Remove trailing newline
    if (!request.body.empty() && request.body.back() == '\n') {
        request.body.pop_back();
    }
    
    return request;
}

std::string HttpServer::formatHttpResponse(const HttpResponse& response) {
    std::ostringstream oss;
    
    // Status line
    oss << "HTTP/1.1 " << response.status_code << " " << response.status_message << "\r\n";
    
    // Headers
    for (const auto& [name, value] : response.headers) {
        oss << name << ": " << value << "\r\n";
    }
    
    // Add server header
    oss << "Server: C++-Database-API/1.0\r\n";
    
    // End of headers
    oss << "\r\n";
    
    // Body
    oss << response.body;
    
    return oss.str();
}

void HttpServer::serverLoop() {
    Logger::info("HTTP server loop started");
    
    if (!initializeServerSocket()) {
        Logger::error("Failed to initialize server socket");
        running_ = false;
        return;
    }
    
    while (running_) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        SocketHandle client_socket = accept(server_socket_, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        
        if (client_socket == kInvalidSocket) {
            if (running_) {
                Logger::warn("Accept failed");
            }
            continue;
        }
        
        handleClient(client_socket);
    }
    
    Logger::info("HTTP server loop stopped");
}

void HttpServer::handleClient(SocketHandle client_socket) {
    // Handle individual client connection
    constexpr int kBufferSize = 4096;
    std::string request_data;
    std::vector<char> buffer(kBufferSize);
    size_t expected_total_size = 0;
    
    auto parseContentLength = [](const std::string& headers) -> size_t {
        std::istringstream stream(headers);
        std::string line;
        while (std::getline(stream, line)) {
            std::string lower_line = line;
            std::transform(lower_line.begin(), lower_line.end(), lower_line.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            const std::string key = "content-length:";
            if (lower_line.rfind(key, 0) == 0) {
                std::string value = line.substr(key.size());
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                try {
                    return static_cast<size_t>(std::stoul(value));
                } catch (...) {
                    return 0;
                }
            }
        }
        return 0;
    };
    
    while (true) {
        int received = recv(client_socket, buffer.data(), static_cast<int>(buffer.size()), 0);
        if (received <= 0) {
            break;
        }
        
        request_data.append(buffer.data(), received);
        
        if (expected_total_size == 0) {
            size_t header_end = request_data.find("\r\n\r\n");
            if (header_end != std::string::npos) {
                std::string headers = request_data.substr(0, header_end);
                size_t content_length = parseContentLength(headers);
                expected_total_size = header_end + 4 + content_length;
            }
        }
        
        if (expected_total_size > 0 && request_data.size() >= expected_total_size) {
            break;
        }
    }
    
    if (request_data.empty()) {
        closeSocket(client_socket);
        return;
    }
    
    HttpRequest request = parseHttpRequest(request_data);
    HttpResponse response;
    
    if (isStaticFileRequest(request.path)) {
        response = serveStaticFile(request.path);
    } else {
        response = processRequest(request);
    }
    
    std::string response_text = formatHttpResponse(response);
    size_t total_sent = 0;
    while (total_sent < response_text.size()) {
        int sent = send(client_socket, response_text.data() + total_sent,
            static_cast<int>(response_text.size() - total_sent), 0);
        if (sent <= 0) {
            break;
        }
        total_sent += static_cast<size_t>(sent);
    }
    
    closeSocket(client_socket);
}

bool HttpServer::initializeServerSocket() {
#ifdef _WIN32
    WSADATA wsa_data;
    int wsa_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (wsa_result != 0) {
        Logger::error("WSAStartup failed: {}", wsa_result);
        return false;
    }
#endif
    
    server_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket_ == kInvalidSocket) {
        Logger::error("Failed to create server socket");
        return false;
    }
    
    int reuse = 1;
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR,
        reinterpret_cast<const char*>(&reuse), sizeof(reuse));
    
    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(static_cast<uint16_t>(port_));
    
    if (host_ == "0.0.0.0" || host_ == "localhost") {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr);
    }
    
    if (bind(server_socket_, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        Logger::error("Failed to bind server socket");
        shutdownServerSocket();
        return false;
    }
    
    if (listen(server_socket_, static_cast<int>(thread_pool_size_)) < 0) {
        Logger::error("Failed to listen on server socket");
        shutdownServerSocket();
        return false;
    }
    
    return true;
}

void HttpServer::shutdownServerSocket() {
    if (server_socket_ == kInvalidSocket) {
        return;
    }
    
#ifdef _WIN32
    shutdown(server_socket_, SD_BOTH);
#else
    shutdown(server_socket_, SHUT_RDWR);
#endif
    closeSocket(server_socket_);
    server_socket_ = kInvalidSocket;
    
#ifdef _WIN32
    WSACleanup();
#endif
}

void HttpServer::closeSocket(SocketHandle socket_handle) {
    if (socket_handle == kInvalidSocket) {
        return;
    }
    
#ifdef _WIN32
    closesocket(socket_handle);
#else
    close(socket_handle);
#endif
}

std::vector<std::string> HttpServer::splitPath(const std::string& path) const {
    std::vector<std::string> parts;
    std::istringstream stream(path);
    std::string part;
    
    while (std::getline(stream, part, '/')) {
        if (!part.empty()) {
            parts.push_back(part);
        }
    }
    
    return parts;
}

std::string HttpServer::urlDecode(const std::string& str) const {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int hex = std::stoi(str.substr(i + 1, 2), nullptr, 16);
            result += static_cast<char>(hex);
            i += 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

std::string HttpServer::urlEncode(const std::string& str) const {
    std::ostringstream oss;
    for (char c : str) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            oss << c;
        } else {
            oss << '%' << std::hex << std::uppercase << static_cast<int>(static_cast<unsigned char>(c));
        }
    }
    return oss.str();
}

std::unordered_map<std::string, std::string> HttpServer::parseQueryString(const std::string& query) const {
    std::unordered_map<std::string, std::string> params;
    std::istringstream stream(query);
    std::string pair;
    
    while (std::getline(stream, pair, '&')) {
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = urlDecode(pair.substr(0, eq_pos));
            std::string value = urlDecode(pair.substr(eq_pos + 1));
            params[key] = value;
        }
    }
    
    return params;
}

std::string HttpServer::getMimeType(const std::string& file_path) const {
    size_t dot_pos = file_path.find_last_of('.');
    if (dot_pos != std::string::npos) {
        std::string ext = file_path.substr(dot_pos + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        if (ext == "html" || ext == "htm") return "text/html";
        if (ext == "css") return "text/css";
        if (ext == "js") return "application/javascript";
        if (ext == "json") return "application/json";
        if (ext == "xml") return "application/xml";
        if (ext == "txt") return "text/plain";
        if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
        if (ext == "png") return "image/png";
        if (ext == "gif") return "image/gif";
        if (ext == "svg") return "image/svg+xml";
    }
    
    return "application/octet-stream";
}

HttpResponse HttpServer::serveStaticFile(const std::string& file_path) const {
    if (!std::filesystem::exists(file_path)) {
        return HttpResponse::notFound("File not found: " + file_path);
    }
    
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        return HttpResponse::internalServerError("Cannot read file: " + file_path);
    }
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    HttpResponse response;
    response.setStatus(200);
    response.setContentType(getMimeType(file_path));
    response.setBody(content);
    
    return response;
}

bool HttpServer::isStaticFileRequest(const std::string& path) const {
    return path.find('.') != std::string::npos && path.find("/api/") == std::string::npos;
}

void HttpServer::addCORSHeaders(HttpResponse& response) const {
    if (enable_cors_) {
        response.setHeader("Access-Control-Allow-Origin", cors_origins_);
        response.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, PATCH, OPTIONS");
        response.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    }
}

// JsonUtils implementation
namespace JsonUtils {
    std::string jsonSuccess(const std::string& message) {
        return "{\"success\": true, \"message\": \"" + message + "\"}";
    }
    
    std::string jsonError(const std::string& message) {
        return "{\"success\": false, \"error\": \"" + message + "\"}";
    }
    
    std::string jsonData(const std::string& key, const std::string& value) {
        return "{\"" + key + "\": \"" + value + "\"}";
    }
    
    std::string jsonObject(const std::unordered_map<std::string, std::string>& data) {
        std::ostringstream oss;
        oss << "{";
        bool first = true;
        for (const auto& [key, value] : data) {
            if (!first) oss << ", ";
            oss << "\"" << key << "\": \"" << value << "\"";
            first = false;
        }
        oss << "}";
        return oss.str();
    }
    
    std::string jsonArray(const std::vector<std::string>& items) {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << "\"" << items[i] << "\"";
        }
        oss << "]";
        return oss.str();
    }
    
    std::unordered_map<std::string, std::string> parseJson(const std::string& json_str) {
        std::unordered_map<std::string, std::string> result;
        // Simple JSON parsing - in production, use a proper JSON library
        std::regex pattern("\"([^\"]+)\":\\s*\"([^\"]+)\"");
        std::sregex_iterator iter(json_str.begin(), json_str.end(), pattern);
        std::sregex_iterator end;
        
        for (; iter != end; ++iter) {
            std::smatch match = *iter;
            if (match.size() == 3) {
                result[match[1].str()] = match[2].str();
            }
        }
        
        return result;
    }
    
    std::string getJsonValue(const std::string& json_str, const std::string& key) {
        auto data = parseJson(json_str);
        auto it = data.find(key);
        return (it != data.end()) ? it->second : "";
    }
}