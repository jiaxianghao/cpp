#ifndef ORDER_REPOSITORY_H
#define ORDER_REPOSITORY_H

#include "models/order.h"
#include "database/database_connection.h"
#include <vector>
#include <memory>

class OrderRepository
{
public:
    explicit OrderRepository(std::shared_ptr<DatabaseConnection> connection);
    
    // CRUD operations
    std::vector<Order> getAll();
    Order getById(int id);
    std::vector<Order> getByUserId(int user_id);
    std::vector<Order> getByStatus(const std::string& status);
    
    int create(const Order& order);
    bool update(const Order& order);
    bool deleteById(int id);
    
    // Order items operations
    bool addOrderItem(int order_id, const OrderItem& item);
    bool removeOrderItem(int order_id, int product_id);
    std::vector<OrderItem> getOrderItems(int order_id);
    bool updateOrderItem(int order_id, int product_id, int quantity);
    
    // Status operations
    bool updateStatus(int order_id, const std::string& status);
    bool cancelOrder(int order_id);
    bool confirmOrder(int order_id);
    bool shipOrder(int order_id);
    bool deliverOrder(int order_id);
    
    // Search operations
    std::vector<Order> searchByDateRange(const std::string& start_date, const std::string& end_date);
    std::vector<Order> searchByAmountRange(double min_amount, double max_amount);
    
    // Statistics
    int getCount();
    int getCountByUserId(int user_id);
    int getCountByStatus(const std::string& status);
    double getTotalAmountByUserId(int user_id);
    double getTotalAmountByDateRange(const std::string& start_date, const std::string& end_date);
    bool exists(int id);

private:
    std::shared_ptr<DatabaseConnection> connection_;
    
    Order mapRowToOrder(const std::map<std::string, std::string>& row);
    OrderItem mapRowToOrderItem(const std::map<std::string, std::string>& row);
    void loadOrderItems(Order& order);
};

#endif // ORDER_REPOSITORY_H

