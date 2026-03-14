#include "models/product.h"
#include <sstream>
#include <regex>

Product::Product(int id, const std::string& name, const std::string& description, 
                 double price, int stock_quantity)
    : id_(id), name_(name), description_(description), 
      price_(price), stock_quantity_(stock_quantity)
{
}

Product::Product(const std::string& name, const std::string& description, 
                 double price, int stock_quantity)
    : name_(name), description_(description), 
      price_(price), stock_quantity_(stock_quantity)
{
}

bool Product::isValid() const
{
    if (name_.empty() || description_.empty())
    {
        return false;
    }
    
    if (price_ <= 0.0)
    {
        return false;
    }
    
    if (stock_quantity_ < 0)
    {
        return false;
    }
    
    return true;
}

bool Product::isInStock() const
{
    return stock_quantity_ > 0;
}

bool Product::canPurchase(int quantity) const
{
    return quantity > 0 && quantity <= stock_quantity_;
}

std::string Product::toString() const
{
    std::ostringstream oss;
    oss << "Product{id=" << id_ 
        << ", name='" << name_ 
        << "', description='" << description_
        << "', price=" << price_
        << ", stock_quantity=" << stock_quantity_
        << ", created_at='" << created_at_ << "'}";
    return oss.str();
}

double Product::calculateTotalPrice(int quantity) const
{
    return price_ * quantity;
}

