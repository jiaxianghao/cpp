#include "UserModule.h"
#include "DatabaseService.h"
#include <iostream>

namespace modules {

bool UserModule::registerUser(const std::string& name, const std::string& email, int age)
{
    // Get DAO from core
    auto& userDAO = core::DatabaseService::getInstance().getUserDAO();

    // Create entity
    core::entities::User user(name, email, age);

    // Business validation
    if (!user.isValid())
    {
        std::cerr << "Invalid user data" << std::endl;
        return false;
    }

    // Check if email already exists
    if (userDAO.findByEmail(email).has_value())
    {
        std::cerr << "Email already registered: " << email << std::endl;
        return false;
    }

    // Save to database
    if (userDAO.create(user))
    {
        std::cout << "User registered successfully: " << name << std::endl;
        return true;
    }

    std::cerr << "Failed to register user" << std::endl;
    return false;
}

std::optional<core::entities::User> UserModule::getUserProfile(int userId)
{
    auto& userDAO = core::DatabaseService::getInstance().getUserDAO();
    return userDAO.findById(userId);
}

std::vector<core::entities::User> UserModule::getActiveUsers()
{
    auto& userDAO = core::DatabaseService::getInstance().getUserDAO();
    return userDAO.findActiveUsers();
}

bool UserModule::updateUserProfile(int userId, const std::string& name, int age)
{
    auto& userDAO = core::DatabaseService::getInstance().getUserDAO();

    // Get existing user
    auto userOpt = userDAO.findById(userId);
    if (!userOpt.has_value())
    {
        std::cerr << "User not found: " << userId << std::endl;
        return false;
    }

    // Update user data
    core::entities::User user = userOpt.value();
    user.name = name;
    user.age = age;

    // Validate
    if (!user.isValid())
    {
        std::cerr << "Invalid user data" << std::endl;
        return false;
    }

    // Save
    return userDAO.update(user);
}

bool UserModule::deactivateUserAccount(int userId)
{
    auto& userDAO = core::DatabaseService::getInstance().getUserDAO();

    // Check if user exists
    auto userOpt = userDAO.findById(userId);
    if (!userOpt.has_value())
    {
        std::cerr << "User not found: " << userId << std::endl;
        return false;
    }

    // Deactivate
    return userDAO.deactivateUser(userId);
}

std::vector<core::entities::User> UserModule::searchUsersByAge(int minAge, int maxAge)
{
    auto& userDAO = core::DatabaseService::getInstance().getUserDAO();
    return userDAO.findByAge(minAge, maxAge);
}

bool UserModule::isEmailRegistered(const std::string& email)
{
    auto& userDAO = core::DatabaseService::getInstance().getUserDAO();
    return userDAO.findByEmail(email).has_value();
}

int UserModule::getActiveUserCount()
{
    auto& userDAO = core::DatabaseService::getInstance().getUserDAO();
    return userDAO.countActiveUsers();
}

int UserModule::batchRegisterUsers(const std::vector<std::tuple<std::string, std::string, int>>& users)
{
    auto& userDAO = core::DatabaseService::getInstance().getUserDAO();

    std::vector<core::entities::User> validUsers;

    // Validate all users first
    for (const auto& [name, email, age] : users)
    {
        core::entities::User user(name, email, age);
        if (user.isValid() && !userDAO.findByEmail(email).has_value())
        {
            validUsers.push_back(user);
        }
        else
        {
            std::cerr << "Skipping invalid or duplicate user: " << email << std::endl;
        }
    }

    // Batch insert
    if (userDAO.createBatch(validUsers))
    {
        std::cout << "Batch registered " << validUsers.size() << " users" << std::endl;
        return static_cast<int>(validUsers.size());
    }

    std::cerr << "Batch registration failed" << std::endl;
    return 0;
}

} // namespace modules
