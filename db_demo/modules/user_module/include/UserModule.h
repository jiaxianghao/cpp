#ifndef MODULES_USER_MODULE_H
#define MODULES_USER_MODULE_H

/**
 * @file UserModule.h
 * @brief User business logic module
 * @version 1.0.0
 */

#include "entities/User.h"
#include <string>
#include <vector>
#include <optional>

namespace modules {

/**
 * @brief User module - handles user-related business logic
 * 
 * This module demonstrates how business modules should interact
 * with the core/DAO layer without directly accessing the database
 */
class UserModule
{
public:
    /**
     * @brief Register a new user
     * @param name User name
     * @param email User email
     * @param age User age
     * @return true if successful, false otherwise
     */
    bool registerUser(const std::string& name, const std::string& email, int age);

    /**
     * @brief Get user profile by ID
     * @param userId User ID
     * @return Optional User entity
     */
    std::optional<core::entities::User> getUserProfile(int userId);

    /**
     * @brief Get all active users
     * @return Vector of active users
     */
    std::vector<core::entities::User> getActiveUsers();

    /**
     * @brief Update user profile
     * @param userId User ID
     * @param name New name
     * @param age New age
     * @return true if successful, false otherwise
     */
    bool updateUserProfile(int userId, const std::string& name, int age);

    /**
     * @brief Deactivate user account
     * @param userId User ID
     * @return true if successful, false otherwise
     */
    bool deactivateUserAccount(int userId);

    /**
     * @brief Search users by age range
     * @param minAge Minimum age
     * @param maxAge Maximum age
     * @return Vector of users
     */
    std::vector<core::entities::User> searchUsersByAge(int minAge, int maxAge);

    /**
     * @brief Check if email is already registered
     * @param email Email to check
     * @return true if registered, false otherwise
     */
    bool isEmailRegistered(const std::string& email);

    /**
     * @brief Get active user count
     * @return Number of active users
     */
    int getActiveUserCount();

    /**
     * @brief Batch register users
     * @param users Vector of user data (name, email, age)
     * @return Number of successfully registered users
     */
    int batchRegisterUsers(const std::vector<std::tuple<std::string, std::string, int>>& users);
};

} // namespace modules

#endif // MODULES_USER_MODULE_H
