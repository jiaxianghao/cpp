#ifndef ORDER_H
#define ORDER_H

#include <string>
#include <vector>

class OrderItem
{
public:
    OrderItem() = default;
    OrderItem(int product_id, const std::string& product_name, 
              double unit_price, int quantity);
    
    // Getters
    int getProductId() const { return product_id_; }
    const std::string& getProductName() const { return product_name_; }
    double getUnitPrice() const { return unit_price_; }
    int getQuantity() const { return quantity_; }
    double getTotalPrice() const { return unit_price_ * quantity_; }
    
    // Setters
    void setProductId(int product_id) { product_id_ = product_id; }
    void setProductName(const std::string& product_name) { product_name_ = product_name; }
    void setUnitPrice(double unit_price) { unit_price_ = unit_price; }
    void setQuantity(int quantity) { quantity_ = quantity; }
    
    // Utility methods
    bool isValid() const;
    std::string toString() const;

private:
    int product_id_ = -1;
    std::string product_name_;
    double unit_price_ = 0.0;
    int quantity_ = 0;
};

class Order
{
public:
    enum class Status
    {
        PENDING,
        CONFIRMED,
        SHIPPED,
        DELIVERED,
        CANCELLED
    };
    
    Order() = default;
    Order(int id, int user_id, const std::string& status, double total_amount);
    Order(int user_id, const std::vector<OrderItem>& items);
    
    // Getters
    int getId() const { return id_; }
    int getUserId() const { return user_id_; }
    const std::string& getStatus() const { return status_; }
    double getTotalAmount() const { return total_amount_; }
    const std::string& getCreatedAt() const { return created_at_; }
    const std::string& getUpdatedAt() const { return updated_at_; }
    const std::vector<OrderItem>& getItems() const { return items_; }
    
    // Setters
    void setId(int id) { id_ = id; }
    void setUserId(int user_id) { user_id_ = user_id; }
    void setStatus(const std::string& status) { status_ = status; }
    void setTotalAmount(double total_amount) { total_amount_ = total_amount; }
    void setCreatedAt(const std::string& created_at) { created_at_ = created_at; }
    void setUpdatedAt(const std::string& updated_at) { updated_at_ = updated_at; }
    void setItems(const std::vector<OrderItem>& items) { items_ = items; }
    
    // Utility methods
    bool isValid() const;
    bool canCancel() const;
    bool canShip() const;
    std::string toString() const;
    void addItem(const OrderItem& item);
    void removeItem(int product_id);
    void calculateTotal();
    std::string getStatusDisplayName() const;

private:
    int id_ = -1;
    int user_id_ = -1;
    std::string status_ = "PENDING";
    double total_amount_ = 0.0;
    std::string created_at_;
    std::string updated_at_;
    std::vector<OrderItem> items_;
    
    std::string statusToString(Status status) const;
    Status stringToStatus(const std::string& status) const;
};

#endif // ORDER_H

