#ifndef PRODUCT_REPOSITORY_H
#define PRODUCT_REPOSITORY_H

#include "models/product.h"
#include "database/database_connection.h"
#include <vector>
#include <memory>

class ProductRepository
{
public:
    explicit ProductRepository(std::shared_ptr<DatabaseConnection> connection);
    
    // CRUD operations
    std::vector<Product> getAll();
    Product getById(int id);
    Product getByName(const std::string& name);
    
    int create(const Product& product);
    bool update(const Product& product);
    bool deleteById(int id);
    
    // Search operations
    std::vector<Product> searchByName(const std::string& pattern);
    std::vector<Product> searchByDescription(const std::string& pattern);
    std::vector<Product> searchByPriceRange(double min_price, double max_price);
    
    // Stock operations
    bool updateStock(int product_id, int new_quantity);
    bool decreaseStock(int product_id, int quantity);
    bool increaseStock(int product_id, int quantity);
    
    // Statistics
    int getCount();
    bool exists(int id);
    bool existsByName(const std::string& name);
    std::vector<Product> getLowStockProducts(int threshold = 10);
    std::vector<Product> getOutOfStockProducts();

private:
    std::shared_ptr<DatabaseConnection> connection_;
    
    Product mapRowToProduct(const std::map<std::string, std::string>& row);
};

#endif // PRODUCT_REPOSITORY_H

