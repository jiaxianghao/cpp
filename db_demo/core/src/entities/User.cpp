#include "entities/User.h"

namespace core {
namespace entities {

User::User()
    : id(0)
    , age(0)
    , isActive(true)
{
}

User::User(const std::string& name, const std::string& email, int age)
    : id(0)
    , name(name)
    , email(email)
    , age(age)
    , isActive(true)
{
}

db_util::TableRecord User::toRecord() const
{
    db_util::TableRecord record;
    if (id > 0)
    {
        record.set("id", id);
    }
    record.set("name", name);
    record.set("email", email);
    record.set("age", age);
    record.set("is_active", isActive);
    return record;
}

User User::fromRecord(const db_util::TableRecord& record)
{
    User user;
    user.id = record.getInt("id");
    user.name = record.getString("name");
    user.email = record.getString("email");
    user.age = record.getInt("age");
    user.isActive = record.getBool("is_active");
    return user;
}

bool User::isValid() const
{
    return !name.empty() && !email.empty() && age > 0 && age < 150;
}

} // namespace entities
} // namespace core
