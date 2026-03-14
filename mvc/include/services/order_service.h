#ifndef ORDER_SERVICE_H
#define ORDER_SERVICE_H

#include "models/order.h"
#include "repositories/order_repository.h"
#include <vector>
#include <memory>
#include <string>

class OrderService
{
public:
    explicit OrderService(std::shared_ptr<OrderRepository> order_repo);
    
    // Order management
    Order createOrder(int user_id, const std::vector<OrderItem>& items);
    Order getOrderById(int id);
    std::vector<Order> getOrdersByUserId(int user_id);
    std::vector<Order> getOrdersByStatus(const std::string& status);
    std::vector<Order> getAllOrders();
    
    bool updateOrder(const Order& order);
    bool deleteOrder(int id);
    
    // Order status management
    bool confirmOrder(int order_id);
    bool shipOrder(int order_id);
    bool deliverOrder(int order_id);
    bool cancelOrder(int order_id);
    
    // Order items management
    bool addOrderItem(int order_id, const OrderItem& item);
    bool removeOrderItem(int order_id, int product_id);
    bool updateOrderItem(int order_id, int product_id, int quantity);
    std::vector<OrderItem> getOrderItems(int order_id);
    
    // Search functionality
    std::vector<Order> searchOrdersByDateRange(const std::string& start_date, const std::string& end_date);
    std::vector<Order> searchOrdersByAmountRange(double min_amount, double max_amount);
    std::vector<Order> searchOrdersByUser(int user_id);
    
    // Validation and business logic
    bool validateOrder(const Order& order);
    bool validateOrderItems(const std::vector<OrderItem>& items);
    bool canCancelOrder(int order_id);
    bool canShipOrder(int order_id);
    bool canDeliverOrder(int order_id);
    
    // Statistics
    int getOrderCount();
    int getOrderCountByUserId(int user_id);
    int getOrderCountByStatus(const std::string& status);
    double getTotalAmountByUserId(int user_id);
    double getTotalAmountByDateRange(const std::string& start_date, const std::string& end_date);
    std::string getOrderStatistics();
    
    // Utility operations
    bool orderExists(int id);
    std::vector<Order> getPendingOrders();
    std::vector<Order> getConfirmedOrders();
    std::vector<Order> getShippedOrders();
    std::vector<Order> getDeliveredOrders();
    std::vector<Order> getCancelledOrders();
    
    // Error handling
    std::string getLastError() const { return last_error_; }

private:
    std::shared_ptr<OrderRepository> order_repo_;
    std::string last_error_;
    
    void setError(const std::string& error);
    std::string sanitizeInput(const std::string& input);
    void logBusinessOperation(const std::string& operation, bool success);
    bool validateOrderItem(const OrderItem& item);
};

#endif // ORDER_SERVICE_H
