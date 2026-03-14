#include "repositories/order_repository.h"
#include <iostream>

using Param = DatabaseConnection::PreparedParam;

OrderRepository::OrderRepository(std::shared_ptr<DatabaseConnection> connection)
    : connection_(connection)
{
}

std::vector<Order> OrderRepository::getAll()
{
    std::vector<Order> orders;
    std::string query = "SELECT id, user_id, status, total_amount, created_at, updated_at FROM orders ORDER BY created_at DESC";
    
    auto results = connection_->fetchAll(query);
    for (const auto& row : results)
    {
        Order order = mapRowToOrder(row);
        loadOrderItems(order);
        orders.push_back(order);
    }
    
    return orders;
}

Order OrderRepository::getById(int id)
{
    auto result = connection_->fetchOnePrepared(
        "SELECT id, user_id, status, total_amount, created_at, updated_at FROM orders WHERE id = ?",
        { Param::fromInt(id) });
    
    if (result.empty())
    {
        return Order();
    }
    
    Order order = mapRowToOrder(result);
    loadOrderItems(order);
    return order;
}

std::vector<Order> OrderRepository::getByUserId(int user_id)
{
    std::vector<Order> orders;
    auto results = connection_->fetchAllPrepared(
        "SELECT id, user_id, status, total_amount, created_at, updated_at FROM orders WHERE user_id = ? ORDER BY created_at DESC",
        { Param::fromInt(user_id) });
    for (const auto& row : results)
    {
        Order order = mapRowToOrder(row);
        loadOrderItems(order);
        orders.push_back(order);
    }
    
    return orders;
}

std::vector<Order> OrderRepository::getByStatus(const std::string& status)
{
    std::vector<Order> orders;
    auto results = connection_->fetchAllPrepared(
        "SELECT id, user_id, status, total_amount, created_at, updated_at FROM orders WHERE status = ? ORDER BY created_at DESC",
        { Param::fromString(status) });
    for (const auto& row : results)
    {
        Order order = mapRowToOrder(row);
        loadOrderItems(order);
        orders.push_back(order);
    }
    
    return orders;
}

int OrderRepository::create(const Order& order)
{
    if (connection_->executePreparedQuery(
        "INSERT INTO orders (user_id, status, total_amount) VALUES (?, ?, ?)",
        { Param::fromInt(order.getUserId()), Param::fromString(order.getStatus()),
            Param::fromDouble(order.getTotalAmount()) }))
    {
        auto result = connection_->fetchOne("SELECT LAST_INSERT_ID() as id");
        if (!result.empty())
        {
            int order_id = std::stoi(result["id"]);
            
            // 添加订单项
            for (const auto& item : order.getItems())
            {
                addOrderItem(order_id, item);
            }
            
            return order_id;
        }
    }
    
    return -1;
}

bool OrderRepository::update(const Order& order)
{
    return connection_->executePreparedQuery(
        "UPDATE orders SET user_id = ?, status = ?, total_amount = ? WHERE id = ?",
        { Param::fromInt(order.getUserId()), Param::fromString(order.getStatus()),
            Param::fromDouble(order.getTotalAmount()), Param::fromInt(order.getId()) });
}

bool OrderRepository::deleteById(int id)
{
    // 先删除订单项
    connection_->executePreparedQuery(
        "DELETE FROM order_items WHERE order_id = ?",
        { Param::fromInt(id) });
    
    // 再删除订单
    return connection_->executePreparedQuery(
        "DELETE FROM orders WHERE id = ?",
        { Param::fromInt(id) });
}

bool OrderRepository::addOrderItem(int order_id, const OrderItem& item)
{
    return connection_->executePreparedQuery(
        "INSERT INTO order_items (order_id, product_id, product_name, unit_price, quantity) VALUES (?, ?, ?, ?, ?)",
        { Param::fromInt(order_id), Param::fromInt(item.getProductId()),
            Param::fromString(item.getProductName()), Param::fromDouble(item.getUnitPrice()),
            Param::fromInt(item.getQuantity()) });
}

bool OrderRepository::removeOrderItem(int order_id, int product_id)
{
    return connection_->executePreparedQuery(
        "DELETE FROM order_items WHERE order_id = ? AND product_id = ?",
        { Param::fromInt(order_id), Param::fromInt(product_id) });
}

std::vector<OrderItem> OrderRepository::getOrderItems(int order_id)
{
    std::vector<OrderItem> items;
    auto results = connection_->fetchAllPrepared(
        "SELECT product_id, product_name, unit_price, quantity FROM order_items WHERE order_id = ?",
        { Param::fromInt(order_id) });
    for (const auto& row : results)
    {
        items.push_back(mapRowToOrderItem(row));
    }
    
    return items;
}

bool OrderRepository::updateOrderItem(int order_id, int product_id, int quantity)
{
    return connection_->executePreparedQuery(
        "UPDATE order_items SET quantity = ? WHERE order_id = ? AND product_id = ?",
        { Param::fromInt(quantity), Param::fromInt(order_id), Param::fromInt(product_id) });
}

bool OrderRepository::updateStatus(int order_id, const std::string& status)
{
    return connection_->executePreparedQuery(
        "UPDATE orders SET status = ? WHERE id = ?",
        { Param::fromString(status), Param::fromInt(order_id) });
}

bool OrderRepository::cancelOrder(int order_id)
{
    return updateStatus(order_id, "CANCELLED");
}

bool OrderRepository::confirmOrder(int order_id)
{
    return updateStatus(order_id, "CONFIRMED");
}

bool OrderRepository::shipOrder(int order_id)
{
    return updateStatus(order_id, "SHIPPED");
}

bool OrderRepository::deliverOrder(int order_id)
{
    return updateStatus(order_id, "DELIVERED");
}

std::vector<Order> OrderRepository::searchByDateRange(const std::string& start_date, const std::string& end_date)
{
    std::vector<Order> orders;
    auto results = connection_->fetchAllPrepared(
        "SELECT id, user_id, status, total_amount, created_at, updated_at FROM orders WHERE created_at BETWEEN ? AND ? ORDER BY created_at DESC",
        { Param::fromString(start_date), Param::fromString(end_date) });
    for (const auto& row : results)
    {
        Order order = mapRowToOrder(row);
        loadOrderItems(order);
        orders.push_back(order);
    }
    
    return orders;
}

std::vector<Order> OrderRepository::searchByAmountRange(double min_amount, double max_amount)
{
    std::vector<Order> orders;
    auto results = connection_->fetchAllPrepared(
        "SELECT id, user_id, status, total_amount, created_at, updated_at FROM orders WHERE total_amount BETWEEN ? AND ? ORDER BY total_amount DESC",
        { Param::fromDouble(min_amount), Param::fromDouble(max_amount) });
    for (const auto& row : results)
    {
        Order order = mapRowToOrder(row);
        loadOrderItems(order);
        orders.push_back(order);
    }
    
    return orders;
}

int OrderRepository::getCount()
{
    auto result = connection_->fetchOne("SELECT COUNT(*) as count FROM orders");
    if (!result.empty())
    {
        return std::stoi(result["count"]);
    }
    return 0;
}

int OrderRepository::getCountByUserId(int user_id)
{
    auto result = connection_->fetchOnePrepared(
        "SELECT COUNT(*) as count FROM orders WHERE user_id = ?",
        { Param::fromInt(user_id) });
    if (!result.empty())
    {
        return std::stoi(result["count"]);
    }
    return 0;
}

int OrderRepository::getCountByStatus(const std::string& status)
{
    auto result = connection_->fetchOnePrepared(
        "SELECT COUNT(*) as count FROM orders WHERE status = ?",
        { Param::fromString(status) });
    if (!result.empty())
    {
        return std::stoi(result["count"]);
    }
    return 0;
}

double OrderRepository::getTotalAmountByUserId(int user_id)
{
    auto result = connection_->fetchOnePrepared(
        "SELECT SUM(total_amount) as total FROM orders WHERE user_id = ?",
        { Param::fromInt(user_id) });
    if (!result.empty() && !result["total"].empty())
    {
        return std::stod(result["total"]);
    }
    return 0.0;
}

double OrderRepository::getTotalAmountByDateRange(const std::string& start_date, const std::string& end_date)
{
    auto result = connection_->fetchOnePrepared(
        "SELECT SUM(total_amount) as total FROM orders WHERE created_at BETWEEN ? AND ?",
        { Param::fromString(start_date), Param::fromString(end_date) });
    if (!result.empty() && !result["total"].empty())
    {
        return std::stod(result["total"]);
    }
    return 0.0;
}

bool OrderRepository::exists(int id)
{
    auto result = connection_->fetchOnePrepared(
        "SELECT 1 FROM orders WHERE id = ?",
        { Param::fromInt(id) });
    return !result.empty();
}

Order OrderRepository::mapRowToOrder(const std::map<std::string, std::string>& row)
{
    Order order;
    
    if (row.find("id") != row.end())
    {
        order.setId(std::stoi(row.at("id")));
    }
    
    if (row.find("user_id") != row.end())
    {
        order.setUserId(std::stoi(row.at("user_id")));
    }
    
    if (row.find("status") != row.end())
    {
        order.setStatus(row.at("status"));
    }
    
    if (row.find("total_amount") != row.end())
    {
        order.setTotalAmount(std::stod(row.at("total_amount")));
    }
    
    if (row.find("created_at") != row.end())
    {
        order.setCreatedAt(row.at("created_at"));
    }
    
    if (row.find("updated_at") != row.end())
    {
        order.setUpdatedAt(row.at("updated_at"));
    }
    
    return order;
}

OrderItem OrderRepository::mapRowToOrderItem(const std::map<std::string, std::string>& row)
{
    OrderItem item;
    
    if (row.find("product_id") != row.end())
    {
        item.setProductId(std::stoi(row.at("product_id")));
    }
    
    if (row.find("product_name") != row.end())
    {
        item.setProductName(row.at("product_name"));
    }
    
    if (row.find("unit_price") != row.end())
    {
        item.setUnitPrice(std::stod(row.at("unit_price")));
    }
    
    if (row.find("quantity") != row.end())
    {
        item.setQuantity(std::stoi(row.at("quantity")));
    }
    
    return item;
}

void OrderRepository::loadOrderItems(Order& order)
{
    order.setItems(getOrderItems(order.getId()));
}

