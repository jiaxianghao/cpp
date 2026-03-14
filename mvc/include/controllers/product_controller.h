#ifndef PRODUCT_CONTROLLER_H
#define PRODUCT_CONTROLLER_H

#include "models/product.h"
#include "services/product_service.h"
#include <vector>
#include <memory>
#include <string>

class ProductController
{
public:
    explicit ProductController(std::shared_ptr<ProductService> product_service);
    
    // Product management endpoints
    struct CreateProductResult
    {
        bool success;
        Product product;
        std::string error_message;
    };
    
    struct UpdateProductResult
    {
        bool success;
        std::string error_message;
    };
    
    struct DeleteProductResult
    {
        bool success;
        std::string error_message;
    };
    
    // Product operations
    CreateProductResult createProduct(const std::string& name, const std::string& description, 
                                    double price, int stock_quantity);
    Product getProductById(int id);
    Product getProductByName(const std::string& name);
    std::vector<Product> getAllProducts();
    
    UpdateProductResult updateProduct(const Product& product);
    DeleteProductResult deleteProduct(int id);
    
    // Search operations
    std::vector<Product> searchProducts(const std::string& keyword);
    std::vector<Product> searchProductsByName(const std::string& pattern);
    std::vector<Product> searchProductsByDescription(const std::string& pattern);
    std::vector<Product> searchProductsByPriceRange(double min_price, double max_price);
    
    // Stock operations
    bool updateStock(int product_id, int new_quantity);
    bool decreaseStock(int product_id, int quantity);
    bool increaseStock(int product_id, int quantity);
    std::vector<Product> getLowStockProducts(int threshold = 10);
    std::vector<Product> getOutOfStockProducts();
    
    // Utility operations
    bool productExists(int id);
    bool isProductNameAvailable(const std::string& name);
    std::string getProductStatistics();
    
    // Error handling
    std::string getLastError() const;

private:
    std::shared_ptr<ProductService> product_service_;
    std::string last_error_;
    
    void setError(const std::string& error);
    void logOperation(const std::string& operation, bool success);
};

#endif // PRODUCT_CONTROLLER_H
