#ifndef MODULES_ORDER_MODULE_H
#define MODULES_ORDER_MODULE_H

/**
 * @file OrderModule.h
 * @brief Order business logic module
 * @version 1.0.0
 */

#include "entities/Order.h"
#include <string>
#include <vector>
#include <optional>

namespace modules {

/**
 * @brief Order module - handles order-related business logic
 * 
 * This module demonstrates complex business logic with transactions
 */
class OrderModule
{
public:
    /**
     * @brief Create a new order
     * @param userId User ID
     * @param productName Product name
     * @param quantity Quantity
     * @param totalPrice Total price
     * @return true if successful, false otherwise
     */
    bool createOrder(int userId, const std::string& productName, int quantity, double totalPrice);

    /**
     * @brief Get order details by ID
     * @param orderId Order ID
     * @return Optional Order entity
     */
    std::optional<core::entities::Order> getOrderDetails(int orderId);

    /**
     * @brief Get all orders for a user
     * @param userId User ID
     * @return Vector of orders
     */
    std::vector<core::entities::Order> getUserOrders(int userId);

    /**
     * @brief Complete an order
     * @param orderId Order ID
     * @return true if successful, false otherwise
     */
    bool completeOrder(int orderId);

    /**
     * @brief Cancel an order
     * @param orderId Order ID
     * @return true if successful, false otherwise
     */
    bool cancelOrder(int orderId);

    /**
     * @brief Get pending orders
     * @return Vector of pending orders
     */
    std::vector<core::entities::Order> getPendingOrders();

    /**
     * @brief Get completed orders
     * @return Vector of completed orders
     */
    std::vector<core::entities::Order> getCompletedOrders();

    /**
     * @brief Get total spending for a user
     * @param userId User ID
     * @return Total amount spent
     */
    double getUserTotalSpending(int userId);

    /**
     * @brief Get order count by status
     * @param status Order status
     * @return Number of orders
     */
    int getOrderCountByStatus(const std::string& status);
};

} // namespace modules

#endif // MODULES_ORDER_MODULE_H
