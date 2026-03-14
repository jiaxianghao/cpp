#ifndef CORE_ENTITIES_PRODUCT_H
#define CORE_ENTITIES_PRODUCT_H

/**
 * @file Product.h
 * @brief Product entity class definition
 * @version 1.0.0
 */

#include "TableManager.h"
#include <string>

namespace core {
namespace entities {

/**
 * @brief Product entity class
 * 
 * Represents a product in the system
 */
class Product
{
public:
    int id;
    std::string name;
    std::string description;
    double price;
    int stock;
    bool available;

    /**
     * @brief Default constructor
     */
    Product();

    /**
     * @brief Parameterized constructor
     */
    Product(const std::string& name, const std::string& description, double price, int stock);

    /**
     * @brief Convert entity to TableRecord for database operations
     * @return TableRecord representation
     */
    db_util::TableRecord toRecord() const;

    /**
     * @brief Create entity from TableRecord
     * @param record Database record
     * @return Product entity
     */
    static Product fromRecord(const db_util::TableRecord& record);

    /**
     * @brief Validate product data
     * @return true if valid, false otherwise
     */
    bool isValid() const;
};

} // namespace entities
} // namespace core

#endif // CORE_ENTITIES_PRODUCT_H
