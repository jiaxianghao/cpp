#include "controllers/product_controller.h"
#include "utils/logger.h"

ProductController::ProductController(std::shared_ptr<ProductService> product_service)
    : product_service_(product_service)
{
}

ProductController::CreateProductResult ProductController::createProduct(const std::string& name, const std::string& description, 
                                                                       double price, int stock_quantity)
{
    CreateProductResult result;
    result.success = false;
    
    try
    {
        Product product = product_service_->createProduct(name, description, price, stock_quantity);
        
        if (product.getId() > 0)
        {
            result.success = true;
            result.product = product;
            logOperation("createProduct", true);
        }
        else
        {
            result.error_message = product_service_->getLastError();
            setError(result.error_message);
            logOperation("createProduct", false);
        }
    }
    catch (const std::exception& e)
    {
        result.error_message = "Exception: " + std::string(e.what());
        setError(result.error_message);
        logOperation("createProduct", false);
    }
    
    return result;
}

Product ProductController::getProductById(int id)
{
    try
    {
        Product product = product_service_->getProductById(id);
        logOperation("getProductById", product.getId() > 0);
        return product;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("getProductById", false);
        return Product();
    }
}

Product ProductController::getProductByName(const std::string& name)
{
    try
    {
        Product product = product_service_->getProductByName(name);
        logOperation("getProductByName", product.getId() > 0);
        return product;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("getProductByName", false);
        return Product();
    }
}

std::vector<Product> ProductController::getAllProducts()
{
    try
    {
        auto products = product_service_->getAllProducts();
        logOperation("getAllProducts", true);
        return products;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("getAllProducts", false);
        return std::vector<Product>();
    }
}

ProductController::UpdateProductResult ProductController::updateProduct(const Product& product)
{
    UpdateProductResult result;
    result.success = false;
    
    try
    {
        result.success = product_service_->updateProduct(product);
        
        if (result.success)
        {
            logOperation("updateProduct", true);
        }
        else
        {
            result.error_message = product_service_->getLastError();
            setError(result.error_message);
            logOperation("updateProduct", false);
        }
    }
    catch (const std::exception& e)
    {
        result.error_message = "Exception: " + std::string(e.what());
        setError(result.error_message);
        logOperation("updateProduct", false);
    }
    
    return result;
}

ProductController::DeleteProductResult ProductController::deleteProduct(int id)
{
    DeleteProductResult result;
    result.success = false;
    
    try
    {
        result.success = product_service_->deleteProduct(id);
        
        if (result.success)
        {
            logOperation("deleteProduct", true);
        }
        else
        {
            result.error_message = product_service_->getLastError();
            setError(result.error_message);
            logOperation("deleteProduct", false);
        }
    }
    catch (const std::exception& e)
    {
        result.error_message = "Exception: " + std::string(e.what());
        setError(result.error_message);
        logOperation("deleteProduct", false);
    }
    
    return result;
}

std::vector<Product> ProductController::searchProducts(const std::string& keyword)
{
    try
    {
        auto products = product_service_->searchProducts(keyword);
        logOperation("searchProducts", true);
        return products;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("searchProducts", false);
        return std::vector<Product>();
    }
}

std::vector<Product> ProductController::searchProductsByName(const std::string& pattern)
{
    try
    {
        auto products = product_service_->searchProductsByName(pattern);
        logOperation("searchProductsByName", true);
        return products;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("searchProductsByName", false);
        return std::vector<Product>();
    }
}

std::vector<Product> ProductController::searchProductsByDescription(const std::string& pattern)
{
    try
    {
        auto products = product_service_->searchProductsByDescription(pattern);
        logOperation("searchProductsByDescription", true);
        return products;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("searchProductsByDescription", false);
        return std::vector<Product>();
    }
}

std::vector<Product> ProductController::searchProductsByPriceRange(double min_price, double max_price)
{
    try
    {
        auto products = product_service_->searchProductsByPriceRange(min_price, max_price);
        logOperation("searchProductsByPriceRange", true);
        return products;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("searchProductsByPriceRange", false);
        return std::vector<Product>();
    }
}

bool ProductController::updateStock(int product_id, int new_quantity)
{
    try
    {
        bool success = product_service_->updateStock(product_id, new_quantity);
        logOperation("updateStock", success);
        return success;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("updateStock", false);
        return false;
    }
}

bool ProductController::decreaseStock(int product_id, int quantity)
{
    try
    {
        bool success = product_service_->decreaseStock(product_id, quantity);
        logOperation("decreaseStock", success);
        return success;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("decreaseStock", false);
        return false;
    }
}

bool ProductController::increaseStock(int product_id, int quantity)
{
    try
    {
        bool success = product_service_->increaseStock(product_id, quantity);
        logOperation("increaseStock", success);
        return success;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("increaseStock", false);
        return false;
    }
}

std::vector<Product> ProductController::getLowStockProducts(int threshold)
{
    try
    {
        auto products = product_service_->getLowStockProducts(threshold);
        logOperation("getLowStockProducts", true);
        return products;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("getLowStockProducts", false);
        return std::vector<Product>();
    }
}

std::vector<Product> ProductController::getOutOfStockProducts()
{
    try
    {
        auto products = product_service_->getOutOfStockProducts();
        logOperation("getOutOfStockProducts", true);
        return products;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("getOutOfStockProducts", false);
        return std::vector<Product>();
    }
}

bool ProductController::productExists(int id)
{
    try
    {
        return product_service_->productExists(id);
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        return false;
    }
}

bool ProductController::isProductNameAvailable(const std::string& name)
{
    try
    {
        return product_service_->isProductNameAvailable(name);
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        return false;
    }
}

std::string ProductController::getProductStatistics()
{
    try
    {
        return product_service_->getProductStatistics();
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        return "Error retrieving statistics";
    }
}

std::string ProductController::getLastError() const
{
    return last_error_;
}

void ProductController::setError(const std::string& error)
{
    last_error_ = error;
    Logger::error("ProductController Error: {}", error);
}

void ProductController::logOperation(const std::string& operation, bool success)
{
    if (success)
    {
        Logger::info("Product operation succeeded: {}", operation);
    }
    else
    {
        Logger::warn("Product operation failed: {}", operation);
    }
}
