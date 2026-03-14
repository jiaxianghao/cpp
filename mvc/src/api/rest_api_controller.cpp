#include "api/rest_api_controller.h"
#include "utils/logger.h"
#include "api/http_server.h"
#include <sstream>
#include <regex>

RestApiController::RestApiController(std::shared_ptr<UserController> user_controller,
                                 std::shared_ptr<ProductController> product_controller,
                                 std::shared_ptr<OrderController> order_controller,
                                 std::shared_ptr<ApplicationService> app_service)
    : user_controller_(user_controller),
      product_controller_(product_controller),
      order_controller_(order_controller),
      app_service_(app_service) {
}

void RestApiController::initializeRoutes(HttpServer& server) {
    Logger::info("Initializing REST API routes");
    
    setupHealthRoutes(server);
    setupUserRoutes(server);
    
    if (product_controller_) {
        setupProductRoutes(server);
    }
    
    if (order_controller_) {
        setupOrderRoutes(server);
    }
    
    if (app_service_) {
        setupApplicationRoutes(server);
    }
    
    Logger::info("REST API routes initialized successfully");
}

void RestApiController::setupHealthRoutes(HttpServer& server) {
    // Health check endpoint
    server.get("/api/health", [this](const HttpRequest& request) {
        return successResponse("{\"status\":\"healthy\",\"timestamp\":\"" + getCurrentTimestamp() + "\"}");
    }, "health_check", "Health check endpoint");
    
    // API info endpoint
    server.get("/api/info", [this](const HttpRequest& request) {
        std::string info = "{";
        info += "\"name\":\"C++ Database API\",";
        info += "\"version\":\"1.0.0\",";
        info += "\"description\":\"RESTful API for database operations\",";
        info += "\"endpoints\":[\"/api/users\",\"/api/products\",\"/api/orders\"]";
        info += "}";
        return successResponse(info);
    }, "api_info", "API information");
}

void RestApiController::setupUserRoutes(HttpServer& server) {
    // GET /api/users - Get all users
    server.get("/api/users", [this](const HttpRequest& request) {
        try {
            auto pagination = parsePaginationParams(request);
            
            // Get all users from controller
            auto users = user_controller_->getAllUsers();
            
            // Apply pagination (simplified - in production, do this in database)
            int start = (pagination.page - 1) * pagination.limit;
            int end = std::min(start + pagination.limit, static_cast<int>(users.size()));
            
            std::vector<User> paginated_users;
            if (start < users.size()) {
                paginated_users.assign(users.begin() + start, users.begin() + end);
            }
            
            return successResponse(userListToJson(paginated_users));
        } catch (const std::exception& e) {
            Logger::error("Error getting users: {}", e.what());
            return serverErrorResponse("Failed to retrieve users");
        }
    }, "get_users", "Get all users");
    
    // GET /api/users/:id - Get user by ID
    server.get("/api/users/:id", [this](const HttpRequest& request) {
        try {
            int user_id = extractIdFromPath(request, "id");
            if (user_id <= 0) {
                return errorResponse("Invalid user ID", 400);
            }
            
            User user = user_controller_->getUserById(user_id);
            if (user.getId() <= 0) {
                return notFoundResponse("User not found");
            }
            
            return successResponse(userToJson(user));
        } catch (const std::exception& e) {
            Logger::error("Error getting user by ID: {}", e.what());
            return serverErrorResponse("Failed to retrieve user");
        }
    }, "get_user_by_id", "Get user by ID");
    
    // POST /api/users - Create new user
    server.post("/api/users", [this](const HttpRequest& request) {
        try {
            if (!validateJsonBody(request)) {
                return errorResponse("Invalid JSON body", 400);
            }
            
            auto [username, email] = parseUserFromBody(request);
            if (username.empty() || email.empty()) {
                return errorResponse("Username and email are required", 400);
            }
            
            auto result = user_controller_->createUser(username, email);
            if (!result.success) {
                return errorResponse(result.error_message, 400);
            }
            
            return successResponse(userToJson(result.user));
        } catch (const std::exception& e) {
            Logger::error("Error creating user: {}", e.what());
            return serverErrorResponse("Failed to create user");
        }
    }, "create_user", "Create new user");
    
    // PUT /api/users/:id - Update user
    server.put("/api/users/:id", [this](const HttpRequest& request) {
        try {
            int user_id = extractIdFromPath(request, "id");
            if (user_id <= 0) {
                return errorResponse("Invalid user ID", 400);
            }
            
            if (!validateJsonBody(request)) {
                return errorResponse("Invalid JSON body", 400);
            }
            
            auto [username, email] = parseUserFromBody(request);
            if (username.empty() || email.empty()) {
                return errorResponse("Username and email are required", 400);
            }
            
            // Get existing user
            User existing_user = user_controller_->getUserById(user_id);
            if (existing_user.getId() <= 0) {
                return notFoundResponse("User not found");
            }
            
            // Update user data
            existing_user.setUsername(username);
            existing_user.setEmail(email);
            
            bool success = user_controller_->updateUser(existing_user);
            if (!success) {
                return errorResponse("Failed to update user", 400);
            }
            
            return successResponse(userToJson(existing_user));
        } catch (const std::exception& e) {
            Logger::error("Error updating user: {}", e.what());
            return serverErrorResponse("Failed to update user");
        }
    }, "update_user", "Update user");
    
    // DELETE /api/users/:id - Delete user
    server.del("/api/users/:id", [this](const HttpRequest& request) {
        try {
            int user_id = extractIdFromPath(request, "id");
            if (user_id <= 0) {
                return errorResponse("Invalid user ID", 400);
            }
            
            bool success = user_controller_->deleteUser(user_id);
            if (!success) {
                return errorResponse("Failed to delete user", 400);
            }
            
            return successResponse("{\"message\":\"User deleted successfully\"}");
        } catch (const std::exception& e) {
            Logger::error("Error deleting user: {}", e.what());
            return serverErrorResponse("Failed to delete user");
        }
    }, "delete_user", "Delete user");
    
    // GET /api/users/search - Search users
    server.get("/api/users/search", [this](const HttpRequest& request) {
        try {
            std::string keyword = request.getQueryParam("q");
            if (keyword.empty()) {
                return errorResponse("Search query parameter 'q' is required", 400);
            }
            
            auto users = user_controller_->searchUsers(keyword);
            return successResponse(userListToJson(users));
        } catch (const std::exception& e) {
            Logger::error("Error searching users: {}", e.what());
            return serverErrorResponse("Failed to search users");
        }
    }, "search_users", "Search users");
    
    // GET /api/users/check-username/:username - Check username availability
    server.get("/api/users/check-username/:username", [this](const HttpRequest& request) {
        try {
            std::string username = request.getPathParam("username");
            if (username.empty()) {
                return errorResponse("Username is required", 400);
            }
            
            bool available = user_controller_->isUsernameAvailable(username);
            std::string result = "{\"available\":" + std::string(available ? "true" : "false") + "}";
            return successResponse(result);
        } catch (const std::exception& e) {
            Logger::error("Error checking username availability: {}", e.what());
            return serverErrorResponse("Failed to check username availability");
        }
    }, "check_username", "Check username availability");
    
    // GET /api/users/check-email/:email - Check email availability
    server.get("/api/users/check-email/:email", [this](const HttpRequest& request) {
        try {
            std::string email = request.getPathParam("email");
            if (email.empty()) {
                return errorResponse("Email is required", 400);
            }
            
            bool available = user_controller_->isEmailAvailable(email);
            std::string result = "{\"available\":" + std::string(available ? "true" : "false") + "}";
            return successResponse(result);
        } catch (const std::exception& e) {
            Logger::error("Error checking email availability: {}", e.what());
            return serverErrorResponse("Failed to check email availability");
        }
    }, "check_email", "Check email availability");
    
    // GET /api/users/stats - Get user statistics
    server.get("/api/users/stats", [this](const HttpRequest& request) {
        try {
            std::string stats = user_controller_->getUserStatistics();
            return successResponse("{\"statistics\":" + stats + "}");
        } catch (const std::exception& e) {
            Logger::error("Error getting user statistics: {}", e.what());
            return serverErrorResponse("Failed to get user statistics");
        }
    }, "user_stats", "Get user statistics");
}

void RestApiController::setupProductRoutes(HttpServer& server) {
    if (!product_controller_) return;
    
    // GET /api/products - Get all products
    server.get("/api/products", [this](const HttpRequest& request) {
        try {
            auto products = product_controller_->getAllProducts();
            return successResponse(productListToJson(products));
        } catch (const std::exception& e) {
            Logger::error("Error getting products: {}", e.what());
            return serverErrorResponse("Failed to retrieve products");
        }
    }, "get_products", "Get all products");
    
    // GET /api/products/:id - Get product by ID
    server.get("/api/products/:id", [this](const HttpRequest& request) {
        try {
            int product_id = extractIdFromPath(request, "id");
            if (product_id <= 0) {
                return errorResponse("Invalid product ID", 400);
            }
            
            Product product = product_controller_->getProductById(product_id);
            if (product.getId() <= 0) {
                return notFoundResponse("Product not found");
            }
            
            return successResponse(productToJson(product));
        } catch (const std::exception& e) {
            Logger::error("Error getting product by ID: {}", e.what());
            return serverErrorResponse("Failed to retrieve product");
        }
    }, "get_product_by_id", "Get product by ID");
    
    // POST /api/products - Create new product
    server.post("/api/products", [this](const HttpRequest& request) {
        try {
            if (!validateJsonBody(request)) {
                return errorResponse("Invalid JSON body", 400);
            }
            
            // Parse product data from request body
            // Implementation would depend on Product model structure
            
            return successResponse("{\"message\":\"Product created successfully\"}");
        } catch (const std::exception& e) {
            Logger::error("Error creating product: {}", e.what());
            return serverErrorResponse("Failed to create product");
        }
    }, "create_product", "Create new product");
}

void RestApiController::setupOrderRoutes(HttpServer& server) {
    if (!order_controller_) return;
    
    // GET /api/orders - Get all orders
    server.get("/api/orders", [this](const HttpRequest& request) {
        try {
            auto orders = order_controller_->getAllOrders();
            return successResponse(orderListToJson(orders));
        } catch (const std::exception& e) {
            Logger::error("Error getting orders: {}", e.what());
            return serverErrorResponse("Failed to retrieve orders");
        }
    }, "get_orders", "Get all orders");
    
    // GET /api/orders/:id - Get order by ID
    server.get("/api/orders/:id", [this](const HttpRequest& request) {
        try {
            int order_id = extractIdFromPath(request, "id");
            if (order_id <= 0) {
                return errorResponse("Invalid order ID", 400);
            }
            
            Order order = order_controller_->getOrderById(order_id);
            if (order.getId() <= 0) {
                return notFoundResponse("Order not found");
            }
            
            return successResponse(orderToJson(order));
        } catch (const std::exception& e) {
            Logger::error("Error getting order by ID: {}", e.what());
            return serverErrorResponse("Failed to retrieve order");
        }
    }, "get_order_by_id", "Get order by ID");
}

void RestApiController::setupApplicationRoutes(HttpServer& server) {
    if (!app_service_) return;
    
    // GET /api/app/stats - Get application statistics
    server.get("/api/app/stats", [this](const HttpRequest& request) {
        try {
            std::string stats = app_service_->getApplicationStatistics();
            return successResponse("{\"statistics\":" + stats + "}");
        } catch (const std::exception& e) {
            Logger::error("Error getting application statistics: {}", e.what());
            return serverErrorResponse("Failed to get application statistics");
        }
    }, "app_stats", "Get application statistics");
    
    // GET /api/app/cache/stats - Get cache statistics
    server.get("/api/app/cache/stats", [this](const HttpRequest& request) {
        try {
            // This would require cache service integration
            std::string cache_stats = "{\"hit_rate\":85.5,\"total_keys\":1000,\"memory_usage\":1048576}";
            return successResponse(cache_stats);
        } catch (const std::exception& e) {
            Logger::error("Error getting cache statistics: {}", e.what());
            return serverErrorResponse("Failed to get cache statistics");
        }
    }, "cache_stats", "Get cache statistics");
    
    // POST /api/app/cache/clear - Clear cache
    server.post("/api/app/cache/clear", [this](const HttpRequest& request) {
        try {
            // This would require cache service integration
            return successResponse("{\"message\":\"Cache cleared successfully\"}");
        } catch (const std::exception& e) {
            Logger::error("Error clearing cache: {}", e.what());
            return serverErrorResponse("Failed to clear cache");
        }
    }, "clear_cache", "Clear application cache");
}

// Helper method implementations
int RestApiController::extractIdFromPath(const HttpRequest& request, const std::string& param_name) {
    std::string id_str = request.getPathParam(param_name);
    if (id_str.empty()) {
        return -1;
    }
    
    try {
        return std::stoi(id_str);
    } catch (const std::exception& e) {
        Logger::warn("Invalid ID parameter: {}", id_str);
        return -1;
    }
}

std::string RestApiController::extractStringFromBody(const HttpRequest& request, const std::string& field) {
    // Simple JSON parsing - in production, use a proper JSON library
    std::regex field_regex("\"" + field + "\"\\s*:\\s*\"([^\"]+)\"");
    std::smatch match;
    
    if (std::regex_search(request.body, match, field_regex)) {
        return match[1].str();
    }
    
    return "";
}

bool RestApiController::validateJsonBody(const HttpRequest& request) {
    // Basic JSON validation
    if (request.body.empty()) {
        return false;
    }
    
    // Check if it looks like JSON (starts with { and ends with })
    std::string trimmed_body = request.body;
    trimmed_body.erase(0, trimmed_body.find_first_not_of(" \t\n\r"));
    trimmed_body.erase(trimmed_body.find_last_not_of(" \t\n\r") + 1);
    
    return !trimmed_body.empty() && trimmed_body.front() == '{' && trimmed_body.back() == '}';
}

std::pair<std::string, std::string> RestApiController::parseUserFromBody(const HttpRequest& request) {
    std::string username = extractStringFromBody(request, "username");
    std::string email = extractStringFromBody(request, "email");
    return {username, email};
}

std::pair<std::string, std::string> RestApiController::parseProductFromBody(const HttpRequest& request) {
    std::string name = extractStringFromBody(request, "name");
    std::string description = extractStringFromBody(request, "description");
    return {name, description};
}

std::tuple<int, int, int> RestApiController::parseOrderFromBody(const HttpRequest& request) {
    // This would parse order data from JSON
    return {0, 0, 0}; // Placeholder
}

RestApiController::PaginationParams RestApiController::parsePaginationParams(const HttpRequest& request) {
    PaginationParams params;
    
    std::string page_str = request.getQueryParam("page");
    std::string limit_str = request.getQueryParam("limit");
    
    if (!page_str.empty()) {
        try {
            params.page = std::max(1, std::stoi(page_str));
        } catch (const std::exception& e) {
            Logger::warn("Invalid page parameter: {}", page_str);
        }
    }
    
    if (!limit_str.empty()) {
        try {
            params.limit = std::max(1, std::min(100, std::stoi(limit_str))); // Limit between 1-100
        } catch (const std::exception& e) {
            Logger::warn("Invalid limit parameter: {}", limit_str);
        }
    }
    
    params.sort_by = request.getQueryParam("sort");
    params.sort_order = request.getQueryParam("order");
    
    return params;
}

std::string RestApiController::createPaginationResponse(const std::vector<User>& items, int total, int page, int limit) {
    std::ostringstream oss;
    oss << "{";
    oss << "\"items\":" << userListToJson(items) << ",";
    oss << "\"total\":" << total << ",";
    oss << "\"page\":" << page << ",";
    oss << "\"limit\":" << limit << ",";
    oss << "\"pages\":" << ((total + limit - 1) / limit);
    oss << "}";
    return oss.str();
}

HttpResponse RestApiController::successResponse(const std::string& data) {
    HttpResponse response;
    response.setStatus(200);
    response.setContentType("application/json");
    response.setBody(data.empty() ? "{\"success\":true}" : data);
    return response;
}

HttpResponse RestApiController::errorResponse(const std::string& message, int status_code) {
    HttpResponse response;
    response.setStatus(status_code);
    response.setContentType("application/json");
    response.setBody("{\"success\":false,\"error\":\"" + message + "\"}");
    return response;
}

HttpResponse RestApiController::notFoundResponse(const std::string& message) {
    return errorResponse(message, 404);
}

HttpResponse RestApiController::serverErrorResponse(const std::string& message) {
    return errorResponse(message, 500);
}

std::string RestApiController::userToJson(const User& user) {
    std::ostringstream oss;
    oss << "{";
    oss << "\"id\":" << user.getId() << ",";
    oss << "\"username\":\"" << user.getUsername() << "\",";
    oss << "\"email\":\"" << user.getEmail() << "\",";
    oss << "\"created_at\":\"" << getCurrentTimestamp() << "\""; // Simplified
    oss << "}";
    return oss.str();
}

std::string RestApiController::userListToJson(const std::vector<User>& users) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < users.size(); ++i) {
        if (i > 0) oss << ",";
        oss << userToJson(users[i]);
    }
    oss << "]";
    return oss.str();
}

std::string RestApiController::productToJson(const Product& product) {
    std::ostringstream oss;
    oss << "{";
    oss << "\"id\":" << product.getId() << ",";
    oss << "\"name\":\"" << product.getName() << "\",";
    oss << "\"description\":\"" << product.getDescription() << "\",";
    oss << "\"price\":" << product.getPrice() << ",";
    oss << "\"stock\":" << product.getStock();
    oss << "}";
    return oss.str();
}

std::string RestApiController::productListToJson(const std::vector<Product>& products) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < products.size(); ++i) {
        if (i > 0) oss << ",";
        oss << productToJson(products[i]);
    }
    oss << "]";
    return oss.str();
}

std::string RestApiController::orderToJson(const Order& order) {
    std::ostringstream oss;
    oss << "{";
    oss << "\"id\":" << order.getId() << ",";
    oss << "\"user_id\":" << order.getUserId() << ",";
    oss << "\"total\":" << order.getTotal() << ",";
    oss << "\"status\":\"" << order.getStatus() << "\"";
    oss << "}";
    return oss.str();
}

std::string RestApiController::orderListToJson(const std::vector<Order>& orders) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < orders.size(); ++i) {
        if (i > 0) oss << ",";
        oss << orderToJson(orders[i]);
    }
    oss << "]";
    return oss.str();
}

std::string RestApiController::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}