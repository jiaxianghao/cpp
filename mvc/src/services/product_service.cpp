#include "services/product_service.h"
#include "utils/logger.h"
#include <algorithm>
#include <sstream>

ProductService::ProductService(std::shared_ptr<ProductRepository> product_repo)
    : product_repo_(product_repo)
{
}

Product ProductService::createProduct(const std::string& name, const std::string& description, 
                                    double price, int stock_quantity)
{
    // Input validation
    if (name.empty() || description.empty())
    {
        setError("Product name and description are required");
        return Product();
    }
    
    if (price <= 0.0)
    {
        setError("Product price must be greater than 0");
        return Product();
    }
    
    if (stock_quantity < 0)
    {
        setError("Stock quantity cannot be negative");
        return Product();
    }
    
    // Sanitize inputs
    std::string clean_name = sanitizeInput(name);
    std::string clean_description = sanitizeInput(description);
    
    // Check if product name already exists
    if (!isProductNameAvailable(clean_name))
    {
        setError("Product name already exists");
        return Product();
    }
    
    // Create product object
    Product product(clean_name, clean_description, price, stock_quantity);
    
    // Validate product object
    if (!validateProduct(product))
    {
        setError("Invalid product data");
        return Product();
    }
    
    // Save to database
    int product_id = product_repo_->create(product);
    if (product_id == -1)
    {
        setError("Failed to create product in database");
        return Product();
    }
    
    product.setId(product_id);
    logBusinessOperation("createProduct", true);
    return product;
}

Product ProductService::getProductById(int id)
{
    if (id <= 0)
    {
        setError("Invalid product ID");
        return Product();
    }
    
    Product product = product_repo_->getById(id);
    logBusinessOperation("getProductById", product.getId() > 0);
    return product;
}

Product ProductService::getProductByName(const std::string& name)
{
    if (name.empty())
    {
        setError("Product name cannot be empty");
        return Product();
    }
    
    Product product = product_repo_->getByName(name);
    logBusinessOperation("getProductByName", product.getId() > 0);
    return product;
}

std::vector<Product> ProductService::getAllProducts()
{
    auto products = product_repo_->getAll();
    logBusinessOperation("getAllProducts", true);
    return products;
}

bool ProductService::updateProduct(const Product& product)
{
    if (!validateProduct(product))
    {
        setError("Invalid product data");
        return false;
    }
    
    if (product.getId() <= 0)
    {
        setError("Invalid product ID");
        return false;
    }
    
    // Check if product exists
    if (!product_repo_->exists(product.getId()))
    {
        setError("Product not found");
        return false;
    }
    
    bool success = product_repo_->update(product);
    logBusinessOperation("updateProduct", success);
    return success;
}

bool ProductService::deleteProduct(int id)
{
    if (id <= 0)
    {
        setError("Invalid product ID");
        return false;
    }
    
    if (!product_repo_->exists(id))
    {
        setError("Product not found");
        return false;
    }
    
    bool success = product_repo_->deleteById(id);
    logBusinessOperation("deleteProduct", success);
    return success;
}

std::vector<Product> ProductService::searchProducts(const std::string& keyword)
{
    if (keyword.empty())
    {
        return getAllProducts();
    }
    
    std::string clean_keyword = sanitizeInput(keyword);
    std::vector<Product> results;
    
    // Search by name
    auto name_results = product_repo_->searchByName(clean_keyword);
    results.insert(results.end(), name_results.begin(), name_results.end());
    
    // Search by description
    auto desc_results = product_repo_->searchByDescription(clean_keyword);
    results.insert(results.end(), desc_results.begin(), desc_results.end());
    
    // Remove duplicates
    std::sort(results.begin(), results.end(), [](const Product& a, const Product& b) {
        return a.getId() < b.getId();
    });
    results.erase(std::unique(results.begin(), results.end(), [](const Product& a, const Product& b) {
        return a.getId() == b.getId();
    }), results.end());
    
    logBusinessOperation("searchProducts", true);
    return results;
}

std::vector<Product> ProductService::searchProductsByName(const std::string& pattern)
{
    if (pattern.empty())
    {
        return getAllProducts();
    }
    
    auto products = product_repo_->searchByName(sanitizeInput(pattern));
    logBusinessOperation("searchProductsByName", true);
    return products;
}

std::vector<Product> ProductService::searchProductsByDescription(const std::string& pattern)
{
    if (pattern.empty())
    {
        return getAllProducts();
    }
    
    auto products = product_repo_->searchByDescription(sanitizeInput(pattern));
    logBusinessOperation("searchProductsByDescription", true);
    return products;
}

std::vector<Product> ProductService::searchProductsByPriceRange(double min_price, double max_price)
{
    if (min_price < 0 || max_price < 0 || min_price > max_price)
    {
        setError("Invalid price range");
        return std::vector<Product>();
    }
    
    auto products = product_repo_->searchByPriceRange(min_price, max_price);
    logBusinessOperation("searchProductsByPriceRange", true);
    return products;
}

bool ProductService::updateStock(int product_id, int new_quantity)
{
    if (product_id <= 0)
    {
        setError("Invalid product ID");
        return false;
    }
    
    if (new_quantity < 0)
    {
        setError("Stock quantity cannot be negative");
        return false;
    }
    
    if (!product_repo_->exists(product_id))
    {
        setError("Product not found");
        return false;
    }
    
    bool success = product_repo_->updateStock(product_id, new_quantity);
    logBusinessOperation("updateStock", success);
    return success;
}

bool ProductService::decreaseStock(int product_id, int quantity)
{
    if (product_id <= 0 || quantity <= 0)
    {
        setError("Invalid product ID or quantity");
        return false;
    }
    
    if (!isProductInStock(product_id, quantity))
    {
        setError("Insufficient stock");
        return false;
    }
    
    bool success = product_repo_->decreaseStock(product_id, quantity);
    logBusinessOperation("decreaseStock", success);
    return success;
}

bool ProductService::increaseStock(int product_id, int quantity)
{
    if (product_id <= 0 || quantity <= 0)
    {
        setError("Invalid product ID or quantity");
        return false;
    }
    
    if (!product_repo_->exists(product_id))
    {
        setError("Product not found");
        return false;
    }
    
    bool success = product_repo_->increaseStock(product_id, quantity);
    logBusinessOperation("increaseStock", success);
    return success;
}

bool ProductService::reserveStock(int product_id, int quantity)
{
    if (product_id <= 0 || quantity <= 0)
    {
        setError("Invalid product ID or quantity");
        return false;
    }
    
    if (!isProductInStock(product_id, quantity))
    {
        setError("Insufficient stock for reservation");
        return false;
    }
    
    bool success = product_repo_->decreaseStock(product_id, quantity);
    logBusinessOperation("reserveStock", success);
    return success;
}

bool ProductService::releaseStock(int product_id, int quantity)
{
    if (product_id <= 0 || quantity <= 0)
    {
        setError("Invalid product ID or quantity");
        return false;
    }
    
    bool success = product_repo_->increaseStock(product_id, quantity);
    logBusinessOperation("releaseStock", success);
    return success;
}

std::vector<Product> ProductService::getLowStockProducts(int threshold)
{
    auto products = product_repo_->getLowStockProducts(threshold);
    logBusinessOperation("getLowStockProducts", true);
    return products;
}

std::vector<Product> ProductService::getOutOfStockProducts()
{
    auto products = product_repo_->getOutOfStockProducts();
    logBusinessOperation("getOutOfStockProducts", true);
    return products;
}

std::vector<Product> ProductService::getInStockProducts()
{
    auto all_products = product_repo_->getAll();
    std::vector<Product> in_stock_products;
    
    for (const auto& product : all_products)
    {
        if (product.isInStock())
        {
            in_stock_products.push_back(product);
        }
    }
    
    logBusinessOperation("getInStockProducts", true);
    return in_stock_products;
}

bool ProductService::validateProduct(const Product& product)
{
    if (product.getName().empty() || product.getDescription().empty())
    {
        return false;
    }
    
    if (product.getPrice() <= 0.0)
    {
        return false;
    }
    
    if (product.getStockQuantity() < 0)
    {
        return false;
    }
    
    return product.isValid();
}

bool ProductService::isProductNameAvailable(const std::string& name)
{
    return !product_repo_->existsByName(name);
}

bool ProductService::isProductInStock(int product_id, int quantity)
{
    Product product = product_repo_->getById(product_id);
    return product.canPurchase(quantity);
}

bool ProductService::canPurchase(int product_id, int quantity)
{
    return isProductInStock(product_id, quantity);
}

int ProductService::getProductCount()
{
    return product_repo_->getCount();
}

int ProductService::getInStockCount()
{
    return getInStockProducts().size();
}

int ProductService::getOutOfStockCount()
{
    return getOutOfStockProducts().size();
}

int ProductService::getLowStockCount(int threshold)
{
    return getLowStockProducts(threshold).size();
}

std::string ProductService::getProductStatistics()
{
    int total_products = getProductCount();
    int in_stock = getInStockCount();
    int out_of_stock = getOutOfStockCount();
    int low_stock = getLowStockCount(10);
    
    std::ostringstream oss;
    oss << "Product Statistics:\n";
    oss << "Total products: " << total_products << "\n";
    oss << "In stock: " << in_stock << "\n";
    oss << "Out of stock: " << out_of_stock << "\n";
    oss << "Low stock (≤10): " << low_stock << "\n";
    
    return oss.str();
}

void ProductService::setError(const std::string& error)
{
    last_error_ = error;
    Logger::error("ProductService Error: {}", error);
}

std::string ProductService::sanitizeInput(const std::string& input)
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

void ProductService::logBusinessOperation(const std::string& operation, bool success)
{
    if (success)
    {
        Logger::info("ProductService operation succeeded: {}", operation);
    }
    else
    {
        Logger::warn("ProductService operation failed: {}", operation);
    }
}
