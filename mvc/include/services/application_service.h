#ifndef APPLICATION_SERVICE_H
#define APPLICATION_SERVICE_H

#include "models/user.h"
#include "models/product.h"
#include "models/order.h"
#include "repositories/user_repository.h"
#include "repositories/product_repository.h"
#include "repositories/order_repository.h"
#include <vector>
#include <memory>

/**
 * 应用服务层 - 协调多个实体的业务操作
 * 这是处理跨实体业务逻辑的地方
 */
class ApplicationService
{
public:
    ApplicationService(std::shared_ptr<UserRepository> user_repo,
                      std::shared_ptr<ProductRepository> product_repo,
                      std::shared_ptr<OrderRepository> order_repo);
    
    // 用户相关业务操作
    struct UserProfile
    {
        User user;
        std::vector<Order> orders;
        double total_spent;
        int total_orders;
    };
    
    UserProfile getUserProfile(int user_id);
    bool deleteUserAndOrders(int user_id);
    
    // 产品相关业务操作
    struct ProductInfo
    {
        Product product;
        int total_orders;
        double total_revenue;
    };
    
    ProductInfo getProductInfo(int product_id);
    bool deleteProductAndUpdateOrders(int product_id);
    
    // 订单相关业务操作
    struct OrderSummary
    {
        Order order;
        User user;
        std::vector<Product> products;
    };
    
    OrderSummary getOrderSummary(int order_id);
    bool createOrderWithValidation(int user_id, const std::vector<OrderItem>& items);
    bool cancelOrderWithRefund(int order_id);
    bool processOrderPayment(int order_id);
    
    // 库存管理业务操作
    bool updateProductStock(int product_id, int new_quantity);
    bool reserveProductStock(int product_id, int quantity);
    bool releaseProductStock(int product_id, int quantity);
    std::vector<Product> getLowStockProducts(int threshold = 10);
    
    // 统计和报表业务操作
    struct BusinessStatistics
    {
        int total_users;
        int total_products;
        int total_orders;
        double total_revenue;
        int pending_orders;
        int low_stock_products;
    };
    
    BusinessStatistics getBusinessStatistics();
    std::vector<Order> getOrdersByDateRange(const std::string& start_date, const std::string& end_date);
    std::vector<Product> getTopSellingProducts(int limit = 10);
    std::vector<User> getTopCustomers(int limit = 10);
    
    // 数据一致性业务操作
    bool validateOrderData(const Order& order);
    bool validateProductData(const Product& product);
    bool validateUserData(const User& user);
    
    // 错误处理
    std::string getLastError() const { return last_error_; }

private:
    std::shared_ptr<UserRepository> user_repo_;
    std::shared_ptr<ProductRepository> product_repo_;
    std::shared_ptr<OrderRepository> order_repo_;
    std::string last_error_;
    
    void setError(const std::string& error);
    bool validateOrderItems(const std::vector<OrderItem>& items);
    bool checkProductAvailability(int product_id, int quantity);
    void logBusinessOperation(const std::string& operation, bool success);
};

#endif // APPLICATION_SERVICE_H

