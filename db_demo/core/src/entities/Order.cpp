#include "entities/Order.h"

namespace core {
namespace entities {

Order::Order()
    : id(0)
    , userId(0)
    , quantity(0)
    , totalPrice(0.0)
    , status("pending")
{
}

Order::Order(int userId, const std::string& productName, int quantity, double totalPrice)
    : id(0)
    , userId(userId)
    , productName(productName)
    , quantity(quantity)
    , totalPrice(totalPrice)
    , status("pending")
{
}

db_util::TableRecord Order::toRecord() const
{
    db_util::TableRecord record;
    if (id > 0)
    {
        record.set("id", id);
    }
    record.set("user_id", userId);
    record.set("product_name", productName);
    record.set("quantity", quantity);
    record.set("total_price", totalPrice);
    record.set("status", status);
    return record;
}

Order Order::fromRecord(const db_util::TableRecord& record)
{
    Order order;
    order.id = record.getInt("id");
    order.userId = record.getInt("user_id");
    order.productName = record.getString("product_name");
    order.quantity = record.getInt("quantity");
    order.totalPrice = record.getDouble("total_price");
    order.status = record.getString("status");
    return order;
}

bool Order::isValid() const
{
    return userId > 0 && !productName.empty() && quantity > 0 && totalPrice > 0.0;
}

} // namespace entities
} // namespace core
