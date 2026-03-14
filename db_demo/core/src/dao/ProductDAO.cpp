#include "dao/ProductDAO.h"

namespace core {
namespace dao {

const std::string ProductDAO::TABLE_NAME = "products";

ProductDAO::ProductDAO(db_util::TableManager& tableManager)
    : tableManager_(tableManager)
{
}

bool ProductDAO::create(const entities::Product& product)
{
    return tableManager_.insert(TABLE_NAME, product.toRecord());
}

std::optional<entities::Product> ProductDAO::findById(int productId)
{
    auto record = tableManager_.selectById(TABLE_NAME, "id", productId);
    if (record.hasField("id"))
    {
        return entities::Product::fromRecord(record);
    }
    return std::nullopt;
}

std::vector<entities::Product> ProductDAO::findAll()
{
    auto records = tableManager_.select(TABLE_NAME);
    std::vector<entities::Product> products;
    for (const auto& record : records)
    {
        products.push_back(entities::Product::fromRecord(record));
    }
    return products;
}

bool ProductDAO::update(const entities::Product& product)
{
    return tableManager_.updateById(TABLE_NAME, product.toRecord(), "id", product.id);
}

bool ProductDAO::deleteById(int productId)
{
    return tableManager_.deleteById(TABLE_NAME, "id", productId);
}

std::vector<entities::Product> ProductDAO::findAvailableProducts()
{
    auto records = tableManager_.selectWhere(TABLE_NAME, "available = 1 AND stock > 0");
    std::vector<entities::Product> products;
    for (const auto& record : records)
    {
        products.push_back(entities::Product::fromRecord(record));
    }
    return products;
}

std::vector<entities::Product> ProductDAO::findByPriceRange(double minPrice, double maxPrice)
{
    std::string condition = "price >= " + std::to_string(minPrice) +
                           " AND price <= " + std::to_string(maxPrice);
    auto records = tableManager_.selectWhere(TABLE_NAME, condition);
    std::vector<entities::Product> products;
    for (const auto& record : records)
    {
        products.push_back(entities::Product::fromRecord(record));
    }
    return products;
}

bool ProductDAO::updateStock(int productId, int newStock)
{
    db_util::TableRecord record;
    record.set("stock", newStock);
    return tableManager_.updateById(TABLE_NAME, record, "id", productId);
}

bool ProductDAO::updatePrice(int productId, double newPrice)
{
    db_util::TableRecord record;
    record.set("price", newPrice);
    return tableManager_.updateById(TABLE_NAME, record, "id", productId);
}

std::vector<entities::Product> ProductDAO::findLowStockProducts(int threshold)
{
    std::string condition = "stock <= " + std::to_string(threshold) + " AND stock > 0";
    auto records = tableManager_.selectWhere(TABLE_NAME, condition);
    std::vector<entities::Product> products;
    for (const auto& record : records)
    {
        products.push_back(entities::Product::fromRecord(record));
    }
    return products;
}

int ProductDAO::countAvailableProducts()
{
    return tableManager_.countWhere(TABLE_NAME, "available = 1");
}

} // namespace dao
} // namespace core
