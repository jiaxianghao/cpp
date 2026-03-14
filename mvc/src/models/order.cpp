#include "models/order.h"
#include <sstream>
#include <algorithm>

OrderItem::OrderItem(int product_id, const std::string& product_name, 
                     double unit_price, int quantity)
    : product_id_(product_id), product_name_(product_name), 
      unit_price_(unit_price), quantity_(quantity)
{
}

bool OrderItem::isValid() const
{
    return product_id_ > 0 && !product_name_.empty() && 
           unit_price_ > 0.0 && quantity_ > 0;
}

std::string OrderItem::toString() const
{
    std::ostringstream oss;
    oss << "OrderItem{product_id=" << product_id_
        << ", product_name='" << product_name_
        << "', unit_price=" << unit_price_
        << ", quantity=" << quantity_
        << ", total_price=" << getTotalPrice() << "}";
    return oss.str();
}

Order::Order(int id, int user_id, const std::string& status, double total_amount)
    : id_(id), user_id_(user_id), status_(status), total_amount_(total_amount)
{
}

Order::Order(int user_id, const std::vector<OrderItem>& items)
    : user_id_(user_id), items_(items)
{
    calculateTotal();
}

bool Order::isValid() const
{
    if (user_id_ <= 0)
    {
        return false;
    }
    
    if (items_.empty())
    {
        return false;
    }
    
    for (const auto& item : items_)
    {
        if (!item.isValid())
        {
            return false;
        }
    }
    
    return true;
}

bool Order::canCancel() const
{
    return status_ == "PENDING" || status_ == "CONFIRMED";
}

bool Order::canShip() const
{
    return status_ == "CONFIRMED";
}

std::string Order::toString() const
{
    std::ostringstream oss;
    oss << "Order{id=" << id_
        << ", user_id=" << user_id_
        << ", status='" << status_
        << "', total_amount=" << total_amount_
        << ", created_at='" << created_at_
        << "', items_count=" << items_.size() << "}";
    return oss.str();
}

void Order::addItem(const OrderItem& item)
{
    if (item.isValid())
    {
        items_.push_back(item);
        calculateTotal();
    }
}

void Order::removeItem(int product_id)
{
    items_.erase(
        std::remove_if(items_.begin(), items_.end(),
            [product_id](const OrderItem& item) {
                return item.getProductId() == product_id;
            }),
        items_.end()
    );
    calculateTotal();
}

void Order::calculateTotal()
{
    total_amount_ = 0.0;
    for (const auto& item : items_)
    {
        total_amount_ += item.getTotalPrice();
    }
}

std::string Order::getStatusDisplayName() const
{
    if (status_ == "PENDING") return "待处理";
    if (status_ == "CONFIRMED") return "已确认";
    if (status_ == "SHIPPED") return "已发货";
    if (status_ == "DELIVERED") return "已送达";
    if (status_ == "CANCELLED") return "已取消";
    return "未知状态";
}

std::string Order::statusToString(Status status) const
{
    switch (status)
    {
        case Status::PENDING: return "PENDING";
        case Status::CONFIRMED: return "CONFIRMED";
        case Status::SHIPPED: return "SHIPPED";
        case Status::DELIVERED: return "DELIVERED";
        case Status::CANCELLED: return "CANCELLED";
        default: return "PENDING";
    }
}

Order::Status Order::stringToStatus(const std::string& status) const
{
    if (status == "PENDING") return Status::PENDING;
    if (status == "CONFIRMED") return Status::CONFIRMED;
    if (status == "SHIPPED") return Status::SHIPPED;
    if (status == "DELIVERED") return Status::DELIVERED;
    if (status == "CANCELLED") return Status::CANCELLED;
    return Status::PENDING;
}

