#ifndef CORE_DAO_ORDER_DAO_H
#define CORE_DAO_ORDER_DAO_H

/**
 * @file OrderDAO.h
 * @brief Data Access Object for Order entity
 * @version 1.0.0
 */

#include "entities/Order.h"
#include "TableManager.h"
#include <vector>
#include <optional>

namespace core {
namespace dao {

/**
 * @brief Data Access Object for Order entity
 * 
 * Provides CRUD operations and custom queries for Order table
 */
class OrderDAO
{
public:
    /**
     * @brief Constructor
     * @param tableManager Reference to TableManager instance
     */
    explicit OrderDAO(db_util::TableManager& tableManager);

    /**
     * @brief Create a new order
     * @param order Order entity to create
     * @return true if successful, false otherwise
     */
    bool create(const entities::Order& order);

    /**
     * @brief Find order by ID
     * @param orderId Order ID
     * @return Optional Order entity
     */
    std::optional<entities::Order> findById(int orderId);

    /**
     * @brief Find all orders
     * @return Vector of Order entities
     */
    std::vector<entities::Order> findAll();

    /**
     * @brief Update existing order
     * @param order Order entity with updated data
     * @return true if successful, false otherwise
     */
    bool update(const entities::Order& order);

    /**
     * @brief Delete order by ID
     * @param orderId Order ID
     * @return true if successful, false otherwise
     */
    bool deleteById(int orderId);

    /**
     * @brief Find orders by user ID
     * @param userId User ID
     * @return Vector of Order entities
     */
    std::vector<entities::Order> findByUserId(int userId);

    /**
     * @brief Find orders by status
     * @param status Order status (pending, completed, cancelled)
     * @return Vector of Order entities
     */
    std::vector<entities::Order> findByStatus(const std::string& status);

    /**
     * @brief Update order status
     * @param orderId Order ID
     * @param newStatus New status
     * @return true if successful, false otherwise
     */
    bool updateStatus(int orderId, const std::string& newStatus);

    /**
     * @brief Calculate total amount for user
     * @param userId User ID
     * @return Total amount
     */
    double calculateTotalAmountForUser(int userId);

    /**
     * @brief Count orders by status
     * @param status Order status
     * @return Number of orders
     */
    int countByStatus(const std::string& status);

private:
    db_util::TableManager& tableManager_;
    static const std::string TABLE_NAME;
};

} // namespace dao
} // namespace core

#endif // CORE_DAO_ORDER_DAO_H
