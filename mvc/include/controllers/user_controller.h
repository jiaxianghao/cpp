#ifndef USER_CONTROLLER_H
#define USER_CONTROLLER_H

#include "models/user.h"
#include "services/user_service.h"
#include <vector>
#include <memory>
#include <string>

class UserController
{
public:
    explicit UserController(std::shared_ptr<UserService> user_service);
    
    // User management endpoints
    struct CreateUserResult
    {
        bool success;
        User user;
        std::string error_message;
    };
    
    struct UpdateUserResult
    {
        bool success;
        std::string error_message;
    };
    
    struct DeleteUserResult
    {
        bool success;
        std::string error_message;
    };
    
    // User operations
    CreateUserResult createUser(const std::string& username, const std::string& email);
    User getUserById(int id);
    User getUserByUsername(const std::string& username);
    User getUserByEmail(const std::string& email);
    std::vector<User> getAllUsers();
    
    UpdateUserResult updateUser(const User& user);
    DeleteUserResult deleteUser(int id);
    
    // Search operations
    std::vector<User> searchUsers(const std::string& keyword);
    std::vector<User> searchUsersByUsername(const std::string& pattern);
    std::vector<User> searchUsersByEmail(const std::string& pattern);
    
    // Utility operations
    bool userExists(int id);
    bool isUsernameAvailable(const std::string& username);
    bool isEmailAvailable(const std::string& email);
    std::string getUserStatistics();
    
    // Error handling
    std::string getLastError() const;

private:
    std::shared_ptr<UserService> user_service_;
    std::string last_error_;
    
    void setError(const std::string& error);
    void logOperation(const std::string& operation, bool success);
};

#endif // USER_CONTROLLER_H
