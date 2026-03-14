#ifndef PRODUCT_SERVICE_H
#define PRODUCT_SERVICE_H

#include "models/product.h"
#include "repositories/product_repository.h"
#include <vector>
#include <memory>
#include <string>

class ProductService
{
public:
    explicit ProductService(std::shared_ptr<ProductRepository> product_repo);
    
    // Product management
    Product createProduct(const std::string& name, const std::string& description, 
                         double price, int stock_quantity);
    Product getProductById(int id);
    Product getProductByName(const std::string& name);
    std::vector<Product> getAllProducts();
    
    bool updateProduct(const Product& product);
    bool deleteProduct(int id);
    
    // Search functionality
    std::vector<Product> searchProducts(const std::string& keyword);
    std::vector<Product> searchProductsByName(const std::string& pattern);
    std::vector<Product> searchProductsByDescription(const std::string& pattern);
    std::vector<Product> searchProductsByPriceRange(double min_price, double max_price);
    
    // Stock management
    bool updateStock(int product_id, int new_quantity);
    bool decreaseStock(int product_id, int quantity);
    bool increaseStock(int product_id, int quantity);
    bool reserveStock(int product_id, int quantity);
    bool releaseStock(int product_id, int quantity);
    
    // Stock queries
    std::vector<Product> getLowStockProducts(int threshold = 10);
    std::vector<Product> getOutOfStockProducts();
    std::vector<Product> getInStockProducts();
    
    // Validation and business logic
    bool validateProduct(const Product& product);
    bool isProductNameAvailable(const std::string& name);
    bool isProductInStock(int product_id, int quantity);
    bool canPurchase(int product_id, int quantity);
    
    // Statistics
    int getProductCount();
    int getInStockCount();
    int getOutOfStockCount();
    int getLowStockCount(int threshold = 10);
    std::string getProductStatistics();
    
    // Error handling
    std::string getLastError() const { return last_error_; }

private:
    std::shared_ptr<ProductRepository> product_repo_;
    std::string last_error_;
    
    void setError(const std::string& error);
    std::string sanitizeInput(const std::string& input);
    void logBusinessOperation(const std::string& operation, bool success);
};

#endif // PRODUCT_SERVICE_H
