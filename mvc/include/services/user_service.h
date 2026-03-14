#ifndef USER_SERVICE_H
#define USER_SERVICE_H

#include "models/user.h"
#include "repositories/user_repository.h"
#include "cache/cache_service.h"
#include <vector>
#include <memory>
#include <string>
#include <chrono>

class UserService
{
public:
    explicit UserService(std::shared_ptr<UserRepository> user_repo, 
                        std::shared_ptr<CacheService> cache_service = nullptr);
    
    // User management
    User createUser(const std::string& username, const std::string& email);
    User getUserById(int id);
    User getUserByUsername(const std::string& username);
    User getUserByEmail(const std::string& email);
    std::vector<User> getAllUsers();
    
    bool updateUser(const User& user);
    bool deleteUser(int id);
    
    // Search functionality
    std::vector<User> searchUsers(const std::string& keyword);
    std::vector<User> searchUsersByUsername(const std::string& pattern);
    std::vector<User> searchUsersByEmail(const std::string& pattern);
    
    // Validation and business logic
    bool validateUser(const User& user);
    bool isUsernameAvailable(const std::string& username);
    bool isEmailAvailable(const std::string& email);
    
    // Statistics
    int getUserCount();
    std::string getUserStatistics();
    
    // Cache management
    void invalidateUserCache(int user_id);
    void invalidateUserCache(const std::string& username);
    void invalidateAllUserCache();
    bool isCacheEnabled() const { return cache_service_ != nullptr; }
    
    // Error handling
    std::string getLastError() const { return last_error_; }

private:
    std::shared_ptr<UserRepository> user_repo_;
    std::shared_ptr<CacheService> cache_service_;
    std::string last_error_;
    
    void setError(const std::string& error);
    std::string sanitizeInput(const std::string& input);
    
    // Cache helper methods
    std::string serializeUser(const User& user);
    std::optional<User> deserializeUser(const std::string& data);
    std::string serializeUserList(const std::vector<User>& users);
    std::vector<User> deserializeUserList(const std::string& data);
    
    // Cache key generators
    std::string getUserCacheKey(int user_id);
    std::string getUserCacheKey(const std::string& username);
    std::string getUserListCacheKey(const std::string& list_type);
    std::string getSearchCacheKey(const std::string& search_term);
};

#endif // USER_SERVICE_H
