#include "dao/OrderDAO.h"

namespace core {
namespace dao {

const std::string OrderDAO::TABLE_NAME = "orders";

OrderDAO::OrderDAO(db_util::TableManager& tableManager)
    : tableManager_(tableManager)
{
}

bool OrderDAO::create(const entities::Order& order)
{
    return tableManager_.insert(TABLE_NAME, order.toRecord());
}

std::optional<entities::Order> OrderDAO::findById(int orderId)
{
    auto record = tableManager_.selectById(TABLE_NAME, "id", orderId);
    if (record.hasField("id"))
    {
        return entities::Order::fromRecord(record);
    }
    return std::nullopt;
}

std::vector<entities::Order> OrderDAO::findAll()
{
    auto records = tableManager_.select(TABLE_NAME);
    std::vector<entities::Order> orders;
    for (const auto& record : records)
    {
        orders.push_back(entities::Order::fromRecord(record));
    }
    return orders;
}

bool OrderDAO::update(const entities::Order& order)
{
    return tableManager_.updateById(TABLE_NAME, order.toRecord(), "id", order.id);
}

bool OrderDAO::deleteById(int orderId)
{
    return tableManager_.deleteById(TABLE_NAME, "id", orderId);
}

std::vector<entities::Order> OrderDAO::findByUserId(int userId)
{
    std::string condition = "user_id = " + std::to_string(userId);
    auto records = tableManager_.selectWhere(TABLE_NAME, condition);
    std::vector<entities::Order> orders;
    for (const auto& record : records)
    {
        orders.push_back(entities::Order::fromRecord(record));
    }
    return orders;
}

std::vector<entities::Order> OrderDAO::findByStatus(const std::string& status)
{
    std::string condition = "status = '" + status + "'";
    auto records = tableManager_.selectWhere(TABLE_NAME, condition);
    std::vector<entities::Order> orders;
    for (const auto& record : records)
    {
        orders.push_back(entities::Order::fromRecord(record));
    }
    return orders;
}

bool OrderDAO::updateStatus(int orderId, const std::string& newStatus)
{
    db_util::TableRecord record;
    record.set("status", newStatus);
    return tableManager_.updateById(TABLE_NAME, record, "id", orderId);
}

double OrderDAO::calculateTotalAmountForUser(int userId)
{
    // Build query to sum completed orders
    std::string query = "SELECT SUM(total_price) as total FROM " + TABLE_NAME +
                       " WHERE user_id = " + std::to_string(userId) +
                       " AND status = 'completed'";

    // Note: We need access to DatabaseManager for this
    // This could be improved by adding a method to TableManager
    // For now, we'll calculate manually
    auto orders = findByUserId(userId);
    double total = 0.0;
    for (const auto& order : orders)
    {
        if (order.status == "completed")
        {
            total += order.totalPrice;
        }
    }
    return total;
}

int OrderDAO::countByStatus(const std::string& status)
{
    std::string condition = "status = '" + status + "'";
    return tableManager_.countWhere(TABLE_NAME, condition);
}

} // namespace dao
} // namespace core
