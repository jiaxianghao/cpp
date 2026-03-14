#ifndef CORE_ENTITIES_USER_H
#define CORE_ENTITIES_USER_H

/**
 * @file User.h
 * @brief User entity class definition
 * @version 1.0.0
 */

#include "TableManager.h"
#include <string>

namespace core {
namespace entities {

/**
 * @brief User entity class
 * 
 * Represents a user in the system
 */
class User
{
public:
    int id;
    std::string name;
    std::string email;
    int age;
    bool isActive;

    /**
     * @brief Default constructor
     */
    User();

    /**
     * @brief Parameterized constructor
     */
    User(const std::string& name, const std::string& email, int age);

    /**
     * @brief Convert entity to TableRecord for database operations
     * @return TableRecord representation
     */
    db_util::TableRecord toRecord() const;

    /**
     * @brief Create entity from TableRecord
     * @param record Database record
     * @return User entity
     */
    static User fromRecord(const db_util::TableRecord& record);

    /**
     * @brief Validate user data
     * @return true if valid, false otherwise
     */
    bool isValid() const;
};

} // namespace entities
} // namespace core

#endif // CORE_ENTITIES_USER_H
