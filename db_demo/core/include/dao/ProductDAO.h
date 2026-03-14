#ifndef CORE_DAO_PRODUCT_DAO_H
#define CORE_DAO_PRODUCT_DAO_H

/**
 * @file ProductDAO.h
 * @brief Data Access Object for Product entity
 * @version 1.0.0
 */

#include "entities/Product.h"
#include "TableManager.h"
#include <vector>
#include <optional>

namespace core {
namespace dao {

/**
 * @brief Data Access Object for Product entity
 * 
 * Provides CRUD operations and custom queries for Product table
 */
class ProductDAO
{
public:
    /**
     * @brief Constructor
     * @param tableManager Reference to TableManager instance
     */
    explicit ProductDAO(db_util::TableManager& tableManager);

    /**
     * @brief Create a new product
     * @param product Product entity to create
     * @return true if successful, false otherwise
     */
    bool create(const entities::Product& product);

    /**
     * @brief Find product by ID
     * @param productId Product ID
     * @return Optional Product entity
     */
    std::optional<entities::Product> findById(int productId);

    /**
     * @brief Find all products
     * @return Vector of Product entities
     */
    std::vector<entities::Product> findAll();

    /**
     * @brief Update existing product
     * @param product Product entity with updated data
     * @return true if successful, false otherwise
     */
    bool update(const entities::Product& product);

    /**
     * @brief Delete product by ID
     * @param productId Product ID
     * @return true if successful, false otherwise
     */
    bool deleteById(int productId);

    /**
     * @brief Find all available products
     * @return Vector of available Product entities
     */
    std::vector<entities::Product> findAvailableProducts();

    /**
     * @brief Find products by price range
     * @param minPrice Minimum price
     * @param maxPrice Maximum price
     * @return Vector of Product entities
     */
    std::vector<entities::Product> findByPriceRange(double minPrice, double maxPrice);

    /**
     * @brief Update product stock
     * @param productId Product ID
     * @param newStock New stock quantity
     * @return true if successful, false otherwise
     */
    bool updateStock(int productId, int newStock);

    /**
     * @brief Update product price
     * @param productId Product ID
     * @param newPrice New price
     * @return true if successful, false otherwise
     */
    bool updatePrice(int productId, double newPrice);

    /**
     * @brief Find products with low stock
     * @param threshold Stock threshold
     * @return Vector of Product entities
     */
    std::vector<entities::Product> findLowStockProducts(int threshold);

    /**
     * @brief Count available products
     * @return Number of available products
     */
    int countAvailableProducts();

private:
    db_util::TableManager& tableManager_;
    static const std::string TABLE_NAME;
};

} // namespace dao
} // namespace core

#endif // CORE_DAO_PRODUCT_DAO_H
