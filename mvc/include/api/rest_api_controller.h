#ifndef REST_API_CONTROLLER_H
#define REST_API_CONTROLLER_H

#include "api/http_server.h"
#include "controllers/user_controller.h"
#include "controllers/product_controller.h"
#include "controllers/order_controller.h"
#include "services/application_service.h"
#include <memory>

// RESTful API controller that handles HTTP requests
class RestApiController {
public:
    RestApiController(std::shared_ptr<UserController> user_controller,
                     std::shared_ptr<ProductController> product_controller = nullptr,
                     std::shared_ptr<OrderController> order_controller = nullptr,
                     std::shared_ptr<ApplicationService> app_service = nullptr);
    
    // Initialize API routes
    void initializeRoutes(HttpServer& server);
    
    // User API endpoints
    void setupUserRoutes(HttpServer& server);
    
    // Product API endpoints  
    void setupProductRoutes(HttpServer& server);
    
    // Order API endpoints
    void setupOrderRoutes(HttpServer& server);
    
    // Application API endpoints
    void setupApplicationRoutes(HttpServer& server);
    
    // Health check endpoint
    void setupHealthRoutes(HttpServer& server);
    
private:
    std::shared_ptr<UserController> user_controller_;
    std::shared_ptr<ProductController> product_controller_;
    std::shared_ptr<OrderController> order_controller_;
    std::shared_ptr<ApplicationService> app_service_;
    
    // Helper methods for request/response handling
    int extractIdFromPath(const HttpRequest& request, const std::string& param_name = "id");
    std::string extractStringFromBody(const HttpRequest& request, const std::string& field);
    bool validateJsonBody(const HttpRequest& request);
    
    // Response helpers
    HttpResponse successResponse(const std::string& data = "");
    HttpResponse errorResponse(const std::string& message, int status_code = 400);
    HttpResponse notFoundResponse(const std::string& message = "Resource not found");
    HttpResponse serverErrorResponse(const std::string& message = "Internal server error");
    
    // JSON serialization helpers
    std::string userToJson(const User& user);
    std::string userListToJson(const std::vector<User>& users);
    std::string productToJson(const Product& product);
    std::string productListToJson(const std::vector<Product>& products);
    std::string orderToJson(const Order& order);
    std::string orderListToJson(const std::vector<Order>& orders);
    
    // Request parsing helpers
    std::pair<std::string, std::string> parseUserFromBody(const HttpRequest& request);
    std::pair<std::string, std::string> parseProductFromBody(const HttpRequest& request);
    std::tuple<int, int, int> parseOrderFromBody(const HttpRequest& request);
    
    // Pagination helpers
    struct PaginationParams {
        int page = 1;
        int limit = 10;
        std::string sort_by;
        std::string sort_order;
    };
    
    PaginationParams parsePaginationParams(const HttpRequest& request);
    std::string createPaginationResponse(const std::vector<User>& items, int total, int page, int limit);
};

#endif // REST_API_CONTROLLER_H