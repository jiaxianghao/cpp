#include "OrderModule.h"
#include "DatabaseService.h"
#include <iostream>

namespace modules {

bool OrderModule::createOrder(int userId, const std::string& productName, int quantity, double totalPrice)
{
    auto& orderDAO = core::DatabaseService::getInstance().getOrderDAO();
    auto& userDAO = core::DatabaseService::getInstance().getUserDAO();

    // Validate user exists
    if (!userDAO.findById(userId).has_value())
    {
        std::cerr << "User not found: " << userId << std::endl;
        return false;
    }

    // Create order
    core::entities::Order order(userId, productName, quantity, totalPrice);

    // Business validation
    if (!order.isValid())
    {
        std::cerr << "Invalid order data" << std::endl;
        return false;
    }

    // Save to database
    if (orderDAO.create(order))
    {
        std::cout << "Order created successfully for user " << userId << std::endl;
        return true;
    }

    std::cerr << "Failed to create order" << std::endl;
    return false;
}

std::optional<core::entities::Order> OrderModule::getOrderDetails(int orderId)
{
    auto& orderDAO = core::DatabaseService::getInstance().getOrderDAO();
    return orderDAO.findById(orderId);
}

std::vector<core::entities::Order> OrderModule::getUserOrders(int userId)
{
    auto& orderDAO = core::DatabaseService::getInstance().getOrderDAO();
    return orderDAO.findByUserId(userId);
}

bool OrderModule::completeOrder(int orderId)
{
    auto& orderDAO = core::DatabaseService::getInstance().getOrderDAO();

    // Check if order exists
    auto orderOpt = orderDAO.findById(orderId);
    if (!orderOpt.has_value())
    {
        std::cerr << "Order not found: " << orderId << std::endl;
        return false;
    }

    // Check if order is pending
    if (orderOpt.value().status != "pending")
    {
        std::cerr << "Order is not in pending status: " << orderId << std::endl;
        return false;
    }

    // Update status
    if (orderDAO.updateStatus(orderId, "completed"))
    {
        std::cout << "Order completed: " << orderId << std::endl;
        return true;
    }

    return false;
}

bool OrderModule::cancelOrder(int orderId)
{
    auto& orderDAO = core::DatabaseService::getInstance().getOrderDAO();

    // Check if order exists
    auto orderOpt = orderDAO.findById(orderId);
    if (!orderOpt.has_value())
    {
        std::cerr << "Order not found: " << orderId << std::endl;
        return false;
    }

    // Check if order can be cancelled
    if (orderOpt.value().status == "completed")
    {
        std::cerr << "Cannot cancel completed order: " << orderId << std::endl;
        return false;
    }

    // Update status
    if (orderDAO.updateStatus(orderId, "cancelled"))
    {
        std::cout << "Order cancelled: " << orderId << std::endl;
        return true;
    }

    return false;
}

std::vector<core::entities::Order> OrderModule::getPendingOrders()
{
    auto& orderDAO = core::DatabaseService::getInstance().getOrderDAO();
    return orderDAO.findByStatus("pending");
}

std::vector<core::entities::Order> OrderModule::getCompletedOrders()
{
    auto& orderDAO = core::DatabaseService::getInstance().getOrderDAO();
    return orderDAO.findByStatus("completed");
}

double OrderModule::getUserTotalSpending(int userId)
{
    auto& orderDAO = core::DatabaseService::getInstance().getOrderDAO();
    return orderDAO.calculateTotalAmountForUser(userId);
}

int OrderModule::getOrderCountByStatus(const std::string& status)
{
    auto& orderDAO = core::DatabaseService::getInstance().getOrderDAO();
    return orderDAO.countByStatus(status);
}

} // namespace modules
