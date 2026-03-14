#ifndef PRODUCT_H
#define PRODUCT_H

#include <string>

class Product
{
public:
    Product() = default;
    Product(int id, const std::string& name, const std::string& description, 
            double price, int stock_quantity);
    Product(const std::string& name, const std::string& description, 
            double price, int stock_quantity);
    
    // Getters
    int getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getDescription() const { return description_; }
    double getPrice() const { return price_; }
    int getStockQuantity() const { return stock_quantity_; }
    const std::string& getCreatedAt() const { return created_at_; }
    const std::string& getUpdatedAt() const { return updated_at_; }
    
    // Setters
    void setId(int id) { id_ = id; }
    void setName(const std::string& name) { name_ = name; }
    void setDescription(const std::string& description) { description_ = description; }
    void setPrice(double price) { price_ = price; }
    void setStockQuantity(int stock_quantity) { stock_quantity_ = stock_quantity; }
    void setCreatedAt(const std::string& created_at) { created_at_ = created_at; }
    void setUpdatedAt(const std::string& updated_at) { updated_at_ = updated_at; }
    
    // Utility methods
    bool isValid() const;
    bool isInStock() const;
    bool canPurchase(int quantity) const;
    std::string toString() const;
    double calculateTotalPrice(int quantity) const;

private:
    int id_ = -1;
    std::string name_;
    std::string description_;
    double price_ = 0.0;
    int stock_quantity_ = 0;
    std::string created_at_;
    std::string updated_at_;
};

#endif // PRODUCT_H

