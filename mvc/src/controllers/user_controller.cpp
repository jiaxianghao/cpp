#include "controllers/user_controller.h"
#include "utils/logger.h"

UserController::UserController(std::shared_ptr<UserService> user_service)
    : user_service_(user_service)
{
}

UserController::CreateUserResult UserController::createUser(const std::string& username, const std::string& email)
{
    CreateUserResult result;
    result.success = false;
    
    try
    {
        User user = user_service_->createUser(username, email);
        
        if (user.getId() > 0)
        {
            result.success = true;
            result.user = user;
            logOperation("createUser", true);
        }
        else
        {
            result.error_message = user_service_->getLastError();
            setError(result.error_message);
            logOperation("createUser", false);
        }
    }
    catch (const std::exception& e)
    {
        result.error_message = "Exception: " + std::string(e.what());
        setError(result.error_message);
        logOperation("createUser", false);
    }
    
    return result;
}

User UserController::getUserById(int id)
{
    try
    {
        User user = user_service_->getUserById(id);
        logOperation("getUserById", user.getId() > 0);
        return user;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("getUserById", false);
        return User();
    }
}

User UserController::getUserByUsername(const std::string& username)
{
    try
    {
        User user = user_service_->getUserByUsername(username);
        logOperation("getUserByUsername", user.getId() > 0);
        return user;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("getUserByUsername", false);
        return User();
    }
}

User UserController::getUserByEmail(const std::string& email)
{
    try
    {
        User user = user_service_->getUserByEmail(email);
        logOperation("getUserByEmail", user.getId() > 0);
        return user;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("getUserByEmail", false);
        return User();
    }
}

std::vector<User> UserController::getAllUsers()
{
    try
    {
        auto users = user_service_->getAllUsers();
        logOperation("getAllUsers", true);
        return users;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("getAllUsers", false);
        return std::vector<User>();
    }
}

UserController::UpdateUserResult UserController::updateUser(const User& user)
{
    UpdateUserResult result;
    result.success = false;
    
    try
    {
        result.success = user_service_->updateUser(user);
        
        if (result.success)
        {
            logOperation("updateUser", true);
        }
        else
        {
            result.error_message = user_service_->getLastError();
            setError(result.error_message);
            logOperation("updateUser", false);
        }
    }
    catch (const std::exception& e)
    {
        result.error_message = "Exception: " + std::string(e.what());
        setError(result.error_message);
        logOperation("updateUser", false);
    }
    
    return result;
}

UserController::DeleteUserResult UserController::deleteUser(int id)
{
    DeleteUserResult result;
    result.success = false;
    
    try
    {
        result.success = user_service_->deleteUser(id);
        
        if (result.success)
        {
            logOperation("deleteUser", true);
        }
        else
        {
            result.error_message = user_service_->getLastError();
            setError(result.error_message);
            logOperation("deleteUser", false);
        }
    }
    catch (const std::exception& e)
    {
        result.error_message = "Exception: " + std::string(e.what());
        setError(result.error_message);
        logOperation("deleteUser", false);
    }
    
    return result;
}

std::vector<User> UserController::searchUsers(const std::string& keyword)
{
    try
    {
        auto users = user_service_->searchUsers(keyword);
        logOperation("searchUsers", true);
        return users;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("searchUsers", false);
        return std::vector<User>();
    }
}

std::vector<User> UserController::searchUsersByUsername(const std::string& pattern)
{
    try
    {
        auto users = user_service_->searchUsersByUsername(pattern);
        logOperation("searchUsersByUsername", true);
        return users;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("searchUsersByUsername", false);
        return std::vector<User>();
    }
}

std::vector<User> UserController::searchUsersByEmail(const std::string& pattern)
{
    try
    {
        auto users = user_service_->searchUsersByEmail(pattern);
        logOperation("searchUsersByEmail", true);
        return users;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        logOperation("searchUsersByEmail", false);
        return std::vector<User>();
    }
}

bool UserController::userExists(int id)
{
    try
    {
        User user = user_service_->getUserById(id);
        return user.getId() > 0;
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        return false;
    }
}

bool UserController::isUsernameAvailable(const std::string& username)
{
    try
    {
        return user_service_->isUsernameAvailable(username);
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        return false;
    }
}

bool UserController::isEmailAvailable(const std::string& email)
{
    try
    {
        return user_service_->isEmailAvailable(email);
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        return false;
    }
}

std::string UserController::getUserStatistics()
{
    try
    {
        return user_service_->getUserStatistics();
    }
    catch (const std::exception& e)
    {
        setError("Exception: " + std::string(e.what()));
        return "Error retrieving statistics";
    }
}

std::string UserController::getLastError() const
{
    return last_error_;
}

void UserController::setError(const std::string& error)
{
    last_error_ = error;
    Logger::error("UserController Error: {}", error);
}

void UserController::logOperation(const std::string& operation, bool success)
{
    if (success)
    {
        Logger::info("User operation succeeded: {}", operation);
    }
    else
    {
        Logger::warn("User operation failed: {}", operation);
    }
}
