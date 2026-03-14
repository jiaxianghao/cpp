#ifndef CORE_DAO_USER_DAO_H
#define CORE_DAO_USER_DAO_H

/**
 * @file UserDAO.h
 * @brief Data Access Object for User entity
 * @version 1.0.0
 */

#include "entities/User.h"
#include "TableManager.h"
#include <vector>
#include <optional>

namespace core {
namespace dao {

/**
 * @brief Data Access Object for User entity
 * 
 * Provides CRUD operations and custom queries for User table
 */
class UserDAO
{
public:
    /**
     * @brief Constructor
     * @param tableManager Reference to TableManager instance
     */
    explicit UserDAO(db_util::TableManager& tableManager);

    /**
     * @brief Create a new user
     * @param user User entity to create
     * @return true if successful, false otherwise
     */
    bool create(const entities::User& user);

    /**
     * @brief Find user by ID
     * @param userId User ID
     * @return Optional User entity
     */
    std::optional<entities::User> findById(int userId);

    /**
     * @brief Find all users
     * @return Vector of User entities
     */
    std::vector<entities::User> findAll();

    /**
     * @brief Update existing user
     * @param user User entity with updated data
     * @return true if successful, false otherwise
     */
    bool update(const entities::User& user);

    /**
     * @brief Delete user by ID
     * @param userId User ID
     * @return true if successful, false otherwise
     */
    bool deleteById(int userId);

    /**
     * @brief Find all active users
     * @return Vector of active User entities
     */
    std::vector<entities::User> findActiveUsers();

    /**
     * @brief Find users by age range
     * @param minAge Minimum age
     * @param maxAge Maximum age
     * @return Vector of User entities
     */
    std::vector<entities::User> findByAge(int minAge, int maxAge);

    /**
     * @brief Find user by email
     * @param email User email
     * @return Optional User entity
     */
    std::optional<entities::User> findByEmail(const std::string& email);

    /**
     * @brief Count active users
     * @return Number of active users
     */
    int countActiveUsers();

    /**
     * @brief Create multiple users in a transaction
     * @param users Vector of User entities
     * @return true if successful, false otherwise
     */
    bool createBatch(const std::vector<entities::User>& users);

    /**
     * @brief Update user email
     * @param userId User ID
     * @param newEmail New email address
     * @return true if successful, false otherwise
     */
    bool updateEmail(int userId, const std::string& newEmail);

    /**
     * @brief Deactivate user
     * @param userId User ID
     * @return true if successful, false otherwise
     */
    bool deactivateUser(int userId);

private:
    db_util::TableManager& tableManager_;
    static const std::string TABLE_NAME;
};

} // namespace dao
} // namespace core

#endif // CORE_DAO_USER_DAO_H
