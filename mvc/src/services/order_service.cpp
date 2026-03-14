#include "services/order_service.h"
#include "utils/logger.h"
#include <algorithm>
#include <sstream>

OrderService::OrderService(std::shared_ptr<OrderRepository> order_repo)
    : order_repo_(order_repo)
{
}

Order OrderService::createOrder(int user_id, const std::vector<OrderItem>& items)
{
    // Input validation
    if (user_id <= 0)
    {
        setError("Invalid user ID");
        return Order();
    }
    
    if (items.empty())
    {
        setError("Order must contain at least one item");
        return Order();
    }
    
    // Validate order items
    if (!validateOrderItems(items))
    {
        setError("Invalid order items");
        return Order();
    }
    
    // Create order object
    Order order(user_id, items);
    
    // Validate order object
    if (!validateOrder(order))
    {
        setError("Invalid order data");
        return Order();
    }
    
    // Save to database
    int order_id = order_repo_->create(order);
    if (order_id == -1)
    {
        setError("Failed to create order in database");
        return Order();
    }
    
    order.setId(order_id);
    logBusinessOperation("createOrder", true);
    return order;
}

Order OrderService::getOrderById(int id)
{
    if (id <= 0)
    {
        setError("Invalid order ID");
        return Order();
    }
    
    Order order = order_repo_->getById(id);
    logBusinessOperation("getOrderById", order.getId() > 0);
    return order;
}

std::vector<Order> OrderService::getOrdersByUserId(int user_id)
{
    if (user_id <= 0)
    {
        setError("Invalid user ID");
        return std::vector<Order>();
    }
    
    auto orders = order_repo_->getByUserId(user_id);
    logBusinessOperation("getOrdersByUserId", true);
    return orders;
}

std::vector<Order> OrderService::getOrdersByStatus(const std::string& status)
{
    if (status.empty())
    {
        setError("Status cannot be empty");
        return std::vector<Order>();
    }
    
    auto orders = order_repo_->getByStatus(status);
    logBusinessOperation("getOrdersByStatus", true);
    return orders;
}

std::vector<Order> OrderService::getAllOrders()
{
    auto orders = order_repo_->getAll();
    logBusinessOperation("getAllOrders", true);
    return orders;
}

bool OrderService::updateOrder(const Order& order)
{
    if (!validateOrder(order))
    {
        setError("Invalid order data");
        return false;
    }
    
    if (order.getId() <= 0)
    {
        setError("Invalid order ID");
        return false;
    }
    
    // Check if order exists
    if (!order_repo_->exists(order.getId()))
    {
        setError("Order not found");
        return false;
    }
    
    bool success = order_repo_->update(order);
    logBusinessOperation("updateOrder", success);
    return success;
}

bool OrderService::deleteOrder(int id)
{
    if (id <= 0)
    {
        setError("Invalid order ID");
        return false;
    }
    
    if (!order_repo_->exists(id))
    {
        setError("Order not found");
        return false;
    }
    
    bool success = order_repo_->deleteById(id);
    logBusinessOperation("deleteOrder", success);
    return success;
}

bool OrderService::confirmOrder(int order_id)
{
    if (order_id <= 0)
    {
        setError("Invalid order ID");
        return false;
    }
    
    if (!order_repo_->exists(order_id))
    {
        setError("Order not found");
        return false;
    }
    
    bool success = order_repo_->confirmOrder(order_id);
    logBusinessOperation("confirmOrder", success);
    return success;
}

bool OrderService::shipOrder(int order_id)
{
    if (order_id <= 0)
    {
        setError("Invalid order ID");
        return false;
    }
    
    if (!order_repo_->exists(order_id))
    {
        setError("Order not found");
        return false;
    }
    
    if (!canShipOrder(order_id))
    {
        setError("Order cannot be shipped in current status");
        return false;
    }
    
    bool success = order_repo_->shipOrder(order_id);
    logBusinessOperation("shipOrder", success);
    return success;
}

bool OrderService::deliverOrder(int order_id)
{
    if (order_id <= 0)
    {
        setError("Invalid order ID");
        return false;
    }
    
    if (!order_repo_->exists(order_id))
    {
        setError("Order not found");
        return false;
    }
    
    if (!canDeliverOrder(order_id))
    {
        setError("Order cannot be delivered in current status");
        return false;
    }
    
    bool success = order_repo_->deliverOrder(order_id);
    logBusinessOperation("deliverOrder", success);
    return success;
}

bool OrderService::cancelOrder(int order_id)
{
    if (order_id <= 0)
    {
        setError("Invalid order ID");
        return false;
    }
    
    if (!order_repo_->exists(order_id))
    {
        setError("Order not found");
        return false;
    }
    
    if (!canCancelOrder(order_id))
    {
        setError("Order cannot be cancelled in current status");
        return false;
    }
    
    bool success = order_repo_->cancelOrder(order_id);
    logBusinessOperation("cancelOrder", success);
    return success;
}

bool OrderService::addOrderItem(int order_id, const OrderItem& item)
{
    if (order_id <= 0)
    {
        setError("Invalid order ID");
        return false;
    }
    
    if (!order_repo_->exists(order_id))
    {
        setError("Order not found");
        return false;
    }
    
    if (!validateOrderItem(item))
    {
        setError("Invalid order item");
        return false;
    }
    
    bool success = order_repo_->addOrderItem(order_id, item);
    logBusinessOperation("addOrderItem", success);
    return success;
}

bool OrderService::removeOrderItem(int order_id, int product_id)
{
    if (order_id <= 0 || product_id <= 0)
    {
        setError("Invalid order ID or product ID");
        return false;
    }
    
    if (!order_repo_->exists(order_id))
    {
        setError("Order not found");
        return false;
    }
    
    bool success = order_repo_->removeOrderItem(order_id, product_id);
    logBusinessOperation("removeOrderItem", success);
    return success;
}

bool OrderService::updateOrderItem(int order_id, int product_id, int quantity)
{
    if (order_id <= 0 || product_id <= 0 || quantity <= 0)
    {
        setError("Invalid order ID, product ID, or quantity");
        return false;
    }
    
    if (!order_repo_->exists(order_id))
    {
        setError("Order not found");
        return false;
    }
    
    bool success = order_repo_->updateOrderItem(order_id, product_id, quantity);
    logBusinessOperation("updateOrderItem", success);
    return success;
}

std::vector<OrderItem> OrderService::getOrderItems(int order_id)
{
    if (order_id <= 0)
    {
        setError("Invalid order ID");
        return std::vector<OrderItem>();
    }
    
    auto items = order_repo_->getOrderItems(order_id);
    logBusinessOperation("getOrderItems", true);
    return items;
}

std::vector<Order> OrderService::searchOrdersByDateRange(const std::string& start_date, const std::string& end_date)
{
    if (start_date.empty() || end_date.empty())
    {
        setError("Start date and end date are required");
        return std::vector<Order>();
    }
    
    auto orders = order_repo_->searchByDateRange(start_date, end_date);
    logBusinessOperation("searchOrdersByDateRange", true);
    return orders;
}

std::vector<Order> OrderService::searchOrdersByAmountRange(double min_amount, double max_amount)
{
    if (min_amount < 0 || max_amount < 0 || min_amount > max_amount)
    {
        setError("Invalid amount range");
        return std::vector<Order>();
    }
    
    auto orders = order_repo_->searchByAmountRange(min_amount, max_amount);
    logBusinessOperation("searchOrdersByAmountRange", true);
    return orders;
}

std::vector<Order> OrderService::searchOrdersByUser(int user_id)
{
    if (user_id <= 0)
    {
        setError("Invalid user ID");
        return std::vector<Order>();
    }
    
    auto orders = order_repo_->getByUserId(user_id);
    logBusinessOperation("searchOrdersByUser", true);
    return orders;
}

bool OrderService::validateOrder(const Order& order)
{
    if (order.getUserId() <= 0)
    {
        return false;
    }
    
    if (order.getItems().empty())
    {
        return false;
    }
    
    for (const auto& item : order.getItems())
    {
        if (!validateOrderItem(item))
        {
            return false;
        }
    }
    
    return order.isValid();
}

bool OrderService::validateOrderItems(const std::vector<OrderItem>& items)
{
    if (items.empty())
    {
        return false;
    }
    
    for (const auto& item : items)
    {
        if (!validateOrderItem(item))
        {
            return false;
        }
    }
    
    return true;
}

bool OrderService::canCancelOrder(int order_id)
{
    Order order = order_repo_->getById(order_id);
    return order.canCancel();
}

bool OrderService::canShipOrder(int order_id)
{
    Order order = order_repo_->getById(order_id);
    return order.canShip();
}

bool OrderService::canDeliverOrder(int order_id)
{
    Order order = order_repo_->getById(order_id);
    return order.getStatus() == "SHIPPED";
}

int OrderService::getOrderCount()
{
    return order_repo_->getCount();
}

int OrderService::getOrderCountByUserId(int user_id)
{
    return order_repo_->getCountByUserId(user_id);
}

int OrderService::getOrderCountByStatus(const std::string& status)
{
    return order_repo_->getCountByStatus(status);
}

double OrderService::getTotalAmountByUserId(int user_id)
{
    return order_repo_->getTotalAmountByUserId(user_id);
}

double OrderService::getTotalAmountByDateRange(const std::string& start_date, const std::string& end_date)
{
    return order_repo_->getTotalAmountByDateRange(start_date, end_date);
}

std::string OrderService::getOrderStatistics()
{
    int total_orders = getOrderCount();
    int pending_orders = getOrderCountByStatus("PENDING");
    int confirmed_orders = getOrderCountByStatus("CONFIRMED");
    int shipped_orders = getOrderCountByStatus("SHIPPED");
    int delivered_orders = getOrderCountByStatus("DELIVERED");
    int cancelled_orders = getOrderCountByStatus("CANCELLED");
    
    std::ostringstream oss;
    oss << "Order Statistics:\n";
    oss << "Total orders: " << total_orders << "\n";
    oss << "Pending: " << pending_orders << "\n";
    oss << "Confirmed: " << confirmed_orders << "\n";
    oss << "Shipped: " << shipped_orders << "\n";
    oss << "Delivered: " << delivered_orders << "\n";
    oss << "Cancelled: " << cancelled_orders << "\n";
    
    return oss.str();
}

bool OrderService::orderExists(int id)
{
    return order_repo_->exists(id);
}

std::vector<Order> OrderService::getPendingOrders()
{
    return getOrdersByStatus("PENDING");
}

std::vector<Order> OrderService::getConfirmedOrders()
{
    return getOrdersByStatus("CONFIRMED");
}

std::vector<Order> OrderService::getShippedOrders()
{
    return getOrdersByStatus("SHIPPED");
}

std::vector<Order> OrderService::getDeliveredOrders()
{
    return getOrdersByStatus("DELIVERED");
}

std::vector<Order> OrderService::getCancelledOrders()
{
    return getOrdersByStatus("CANCELLED");
}

void OrderService::setError(const std::string& error)
{
    last_error_ = error;
    Logger::error("OrderService Error: {}", error);
}

std::string OrderService::sanitizeInput(const std::string& input)
{
    std::string result = input;
    
    // Remove leading and trailing whitespace
    result.erase(0, result.find_first_not_of(" \t\n\r"));
    result.erase(result.find_last_not_of(" \t\n\r") + 1);
    
    // Remove potentially dangerous characters
    result.erase(std::remove(result.begin(), result.end(), '\''), result.end());
    result.erase(std::remove(result.begin(), result.end(), '"'), result.end());
    result.erase(std::remove(result.begin(), result.end(), ';'), result.end());
    result.erase(std::remove(result.begin(), result.end(), '\\'), result.end());
    
    return result;
}

void OrderService::logBusinessOperation(const std::string& operation, bool success)
{
    if (success)
    {
        Logger::info("OrderService operation succeeded: {}", operation);
    }
    else
    {
        Logger::warn("OrderService operation failed: {}", operation);
    }
}

bool OrderService::validateOrderItem(const OrderItem& item)
{
    return item.isValid();
}
