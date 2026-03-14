#include "dao/UserDAO.h"

namespace core {
namespace dao {

const std::string UserDAO::TABLE_NAME = "users";

UserDAO::UserDAO(db_util::TableManager& tableManager)
    : tableManager_(tableManager)
{
}

bool UserDAO::create(const entities::User& user)
{
    return tableManager_.insert(TABLE_NAME, user.toRecord());
}

std::optional<entities::User> UserDAO::findById(int userId)
{
    auto record = tableManager_.selectById(TABLE_NAME, "id", userId);
    if (record.hasField("id"))
    {
        return entities::User::fromRecord(record);
    }
    return std::nullopt;
}

std::vector<entities::User> UserDAO::findAll()
{
    auto records = tableManager_.select(TABLE_NAME);
    std::vector<entities::User> users;
    for (const auto& record : records)
    {
        users.push_back(entities::User::fromRecord(record));
    }
    return users;
}

bool UserDAO::update(const entities::User& user)
{
    return tableManager_.updateById(TABLE_NAME, user.toRecord(), "id", user.id);
}

bool UserDAO::deleteById(int userId)
{
    return tableManager_.deleteById(TABLE_NAME, "id", userId);
}

std::vector<entities::User> UserDAO::findActiveUsers()
{
    auto records = tableManager_.selectWhere(TABLE_NAME, "is_active = 1");
    std::vector<entities::User> users;
    for (const auto& record : records)
    {
        users.push_back(entities::User::fromRecord(record));
    }
    return users;
}

std::vector<entities::User> UserDAO::findByAge(int minAge, int maxAge)
{
    std::string condition = "age >= " + std::to_string(minAge) +
                           " AND age <= " + std::to_string(maxAge);
    auto records = tableManager_.selectWhere(TABLE_NAME, condition);
    std::vector<entities::User> users;
    for (const auto& record : records)
    {
        users.push_back(entities::User::fromRecord(record));
    }
    return users;
}

std::optional<entities::User> UserDAO::findByEmail(const std::string& email)
{
    std::string condition = "email = '" + email + "'";
    auto record = tableManager_.selectOne(TABLE_NAME, condition);
    if (record.hasField("id"))
    {
        return entities::User::fromRecord(record);
    }
    return std::nullopt;
}

int UserDAO::countActiveUsers()
{
    return tableManager_.countWhere(TABLE_NAME, "is_active = 1");
}

bool UserDAO::createBatch(const std::vector<entities::User>& users)
{
    std::vector<db_util::TableRecord> records;
    for (const auto& user : users)
    {
        records.push_back(user.toRecord());
    }
    return tableManager_.insertBatch(TABLE_NAME, records);
}

bool UserDAO::updateEmail(int userId, const std::string& newEmail)
{
    db_util::TableRecord record;
    record.set("email", newEmail);
    return tableManager_.updateById(TABLE_NAME, record, "id", userId);
}

bool UserDAO::deactivateUser(int userId)
{
    db_util::TableRecord record;
    record.set("is_active", false);
    return tableManager_.updateById(TABLE_NAME, record, "id", userId);
}

} // namespace dao
} // namespace core
