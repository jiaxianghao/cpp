#include "entities/Product.h"

namespace core {
namespace entities {

Product::Product()
    : id(0)
    , price(0.0)
    , stock(0)
    , available(true)
{
}

Product::Product(const std::string& name, const std::string& description, double price, int stock)
    : id(0)
    , name(name)
    , description(description)
    , price(price)
    , stock(stock)
    , available(true)
{
}

db_util::TableRecord Product::toRecord() const
{
    db_util::TableRecord record;
    if (id > 0)
    {
        record.set("id", id);
    }
    record.set("name", name);
    record.set("description", description);
    record.set("price", price);
    record.set("stock", stock);
    record.set("available", available);
    return record;
}

Product Product::fromRecord(const db_util::TableRecord& record)
{
    Product product;
    product.id = record.getInt("id");
    product.name = record.getString("name");
    product.description = record.getString("description");
    product.price = record.getDouble("price");
    product.stock = record.getInt("stock");
    product.available = record.getBool("available");
    return product;
}

bool Product::isValid() const
{
    return !name.empty() && price >= 0.0 && stock >= 0;
}

} // namespace entities
} // namespace core
