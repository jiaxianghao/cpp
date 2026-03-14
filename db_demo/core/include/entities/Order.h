#ifndef CORE_ENTITIES_ORDER_H
#define CORE_ENTITIES_ORDER_H

/**
 * @file Order.h
 * @brief Order entity class definition
 * @version 1.0.0
 */

#include "TableManager.h"
#include <string>

namespace core {
namespace entities {

/**
 * @brief Order entity class
 * 
 * Represents an order in the system
 */
class Order
{
public:
    int id;
    int userId;
    std::string productName;
    int quantity;
    double totalPrice;
    std::string status; // pending, completed, cancelled

    /**
     * @brief Default constructor
     */
    Order();

    /**
     * @brief Parameterized constructor
     */
    Order(int userId, const std::string& productName, int quantity, double totalPrice);

    /**
     * @brief Convert entity to TableRecord for database operations
     * @return TableRecord representation
     */
    db_util::TableRecord toRecord() const;

    /**
     * @brief Create entity from TableRecord
     * @param record Database record
     * @return Order entity
     */
    static Order fromRecord(const db_util::TableRecord& record);

    /**
     * @brief Validate order data
     * @return true if valid, false otherwise
     */
    bool isValid() const;
};

} // namespace entities
} // namespace core

#endif // CORE_ENTITIES_ORDER_H
