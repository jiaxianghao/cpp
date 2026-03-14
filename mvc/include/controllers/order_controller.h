#ifndef ORDER_CONTROLLER_H
#define ORDER_CONTROLLER_H

#include "models/order.h"
#include "services/order_service.h"
#include <vector>
#include <memory>
#include <string>

class OrderController
{
public:
    explicit OrderController(std::shared_ptr<OrderService> order_service);
    
    // Order management endpoints
    struct CreateOrderResult
    {
        bool success;
        Order order;
        std::string error_message;
    };
    
    struct UpdateOrderResult
    {
        bool success;
        std::string error_message;
    };
    
    struct DeleteOrderResult
    {
        bool success;
        std::string error_message;
    };
    
    struct CancelOrderResult
    {
        bool success;
        std::string error_message;
    };
    
    // Order operations
    CreateOrderResult createOrder(int user_id, const std::vector<OrderItem>& items);
    Order getOrderById(int id);
    std::vector<Order> getOrdersByUserId(int user_id);
    std::vector<Order> getOrdersByStatus(const std::string& status);
    std::vector<Order> getAllOrders();
    
    UpdateOrderResult updateOrder(const Order& order);
    DeleteOrderResult deleteOrder(int id);
    CancelOrderResult cancelOrder(int id);
    
    // Order status operations
    bool confirmOrder(int order_id);
    bool shipOrder(int order_id);
    bool deliverOrder(int order_id);
    
    // Order items operations
    bool addOrderItem(int order_id, const OrderItem& item);
    bool removeOrderItem(int order_id, int product_id);
    bool updateOrderItem(int order_id, int product_id, int quantity);
    std::vector<OrderItem> getOrderItems(int order_id);
    
    // Search operations
    std::vector<Order> searchOrdersByDateRange(const std::string& start_date, const std::string& end_date);
    std::vector<Order> searchOrdersByAmountRange(double min_amount, double max_amount);
    std::vector<Order> searchOrdersByUser(int user_id);
    
    // Statistics
    int getOrderCount();
    int getOrderCountByUserId(int user_id);
    int getOrderCountByStatus(const std::string& status);
    double getTotalAmountByUserId(int user_id);
    double getTotalAmountByDateRange(const std::string& start_date, const std::string& end_date);
    std::string getOrderStatistics();
    
    // Utility operations
    bool orderExists(int id);
    bool canCancelOrder(int order_id);
    bool canShipOrder(int order_id);
    bool canDeliverOrder(int order_id);
    
    // Error handling
    std::string getLastError() const;

private:
    std::shared_ptr<OrderService> order_service_;
    std::string last_error_;
    
    void setError(const std::string& error);
    void logOperation(const std::string& operation, bool success);
};

#endif // ORDER_CONTROLLER_H
