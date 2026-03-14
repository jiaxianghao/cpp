#include "services/user_service.h"
#include "utils/logger.h"
#include <algorithm>
#include <sstream>
#include <json/json.h>

namespace
{
std::string toJsonString(const Json::Value& value)
{
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, value);
}

std::optional<User> userFromJsonValue(const Json::Value& value)
{
    if (!value.isObject())
    {
        return std::nullopt;
    }

    if (!value.isMember("id") || !value.isMember("username") || !value.isMember("email"))
    {
        return std::nullopt;
    }

    User user(value["username"].asString(), value["email"].asString());
    user.setId(value["id"].asInt());
    if (value.isMember("created_at"))
    {
        user.setCreatedAt(value["created_at"].asString());
    }
    return user;
}
}

UserService::UserService(std::shared_ptr<UserRepository> user_repo, 
                       std::shared_ptr<CacheService> cache_service)
    : user_repo_(user_repo), cache_service_(cache_service)
{
}

User UserService::createUser(const std::string& username, const std::string& email)
{
    // Input validation
    if (username.empty() || email.empty())
    {
        setError("Username and email are required");
        return User();
    }
    
    // Sanitize inputs
    std::string clean_username = sanitizeInput(username);
    std::string clean_email = sanitizeInput(email);
    
    // Check if username already exists
    if (!isUsernameAvailable(clean_username))
    {
        setError("Username already exists");
        return User();
    }
    
    // Check if email already exists
    if (!isEmailAvailable(clean_email))
    {
        setError("Email already exists");
        return User();
    }
    
    // Create user object
    User user(clean_username, clean_email);
    
    // Validate user object
    if (!validateUser(user))
    {
        setError("Invalid user data");
        return User();
    }
    
    // Save to database
    int user_id = user_repo_->create(user);
    if (user_id == -1)
    {
        setError("Failed to create user in database");
        return User();
    }
    
    user.setId(user_id);
    
    // Cache the newly created user
    if (cache_service_)
    {
        std::string user_data = serializeUser(user);
        cache_service_->cacheUser(std::to_string(user_id), user_data);
        cache_service_->cacheUser(clean_username, user_data);
        Logger::debug("User cached after creation: {} ({})", clean_username, user_id);
    }
    
    return user;
}

User UserService::getUserById(int id)
{
    if (id <= 0)
    {
        setError("Invalid user ID");
        return User();
    }
    
    // Try to get from cache first
    if (cache_service_)
    {
        std::string user_id_str = std::to_string(id);
        auto cached_data = cache_service_->getCachedUser(user_id_str);
        
        if (cached_data.has_value())
        {
            auto user = deserializeUser(cached_data.value());
            if (user.has_value())
            {
                Logger::debug("User retrieved from cache by ID: {}", id);
                return user.value();
            }
        }
    }
    
    // Cache miss, get from database
    User user = user_repo_->getById(id);
    
    // Cache the result if user found
    if (user.getId() > 0 && cache_service_)
    {
        std::string user_data = serializeUser(user);
        cache_service_->cacheUser(std::to_string(id), user_data);
        cache_service_->cacheUser(user.getUsername(), user_data);
        Logger::debug("User cached after database retrieval: {} ({})", user.getUsername(), id);
    }
    
    return user;
}

User UserService::getUserByUsername(const std::string& username)
{
    if (username.empty())
    {
        setError("Username cannot be empty");
        return User();
    }
    
    return user_repo_->getByUsername(username);
}

User UserService::getUserByEmail(const std::string& email)
{
    if (email.empty())
    {
        setError("Email cannot be empty");
        return User();
    }
    
    return user_repo_->getByEmail(email);
}

std::vector<User> UserService::getAllUsers()
{
    return user_repo_->getAll();
}

bool UserService::updateUser(const User& user)
{
    if (!validateUser(user))
    {
        setError("Invalid user data");
        return false;
    }
    
    if (user.getId() <= 0)
    {
        setError("Invalid user ID");
        return false;
    }
    
    // Check if user exists
    if (!user_repo_->exists(user.getId()))
    {
        setError("User not found");
        return false;
    }
    
    bool success = user_repo_->update(user);
    
    // Invalidate cache if update was successful
    if (success && cache_service_)
    {
        invalidateUserCache(user.getId());
        invalidateUserCache(user.getUsername());
        Logger::debug("User cache invalidated after update: {} ({})", user.getUsername(), user.getId());
    }
    
    return success;
}

bool UserService::deleteUser(int id)
{
    if (id <= 0)
    {
        setError("Invalid user ID");
        return false;
    }
    
    // Get user info before deletion for cache invalidation
    User user = getUserById(id);
    
    if (!user_repo_->exists(id))
    {
        setError("User not found");
        return false;
    }
    
    bool success = user_repo_->deleteById(id);
    
    // Invalidate cache if deletion was successful
    if (success && cache_service_ && user.getId() > 0)
    {
        invalidateUserCache(id);
        invalidateUserCache(user.getUsername());
        Logger::debug("User cache invalidated after deletion: {} ({})", user.getUsername(), id);
    }
    
    return success;
}

std::vector<User> UserService::searchUsers(const std::string& keyword)
{
    if (keyword.empty())
    {
        return getAllUsers();
    }
    
    std::string clean_keyword = sanitizeInput(keyword);
    std::vector<User> results;
    
    // Search by username
    auto username_results = user_repo_->searchByUsername(clean_keyword);
    results.insert(results.end(), username_results.begin(), username_results.end());
    
    // Search by email
    auto email_results = user_repo_->searchByEmail(clean_keyword);
    results.insert(results.end(), email_results.begin(), email_results.end());
    
    // Remove duplicates
    std::sort(results.begin(), results.end(), [](const User& a, const User& b) {
        return a.getId() < b.getId();
    });
    results.erase(std::unique(results.begin(), results.end(), [](const User& a, const User& b) {
        return a.getId() == b.getId();
    }), results.end());
    
    return results;
}

std::vector<User> UserService::searchUsersByUsername(const std::string& pattern)
{
    if (pattern.empty())
    {
        return getAllUsers();
    }
    
    return user_repo_->searchByUsername(sanitizeInput(pattern));
}

std::vector<User> UserService::searchUsersByEmail(const std::string& pattern)
{
    if (pattern.empty())
    {
        return getAllUsers();
    }
    
    return user_repo_->searchByEmail(sanitizeInput(pattern));
}

bool UserService::validateUser(const User& user)
{
    if (user.getUsername().empty() || user.getEmail().empty())
    {
        return false;
    }
    
    return user.isValid();
}

bool UserService::isUsernameAvailable(const std::string& username)
{
    return !user_repo_->existsByUsername(username);
}

bool UserService::isEmailAvailable(const std::string& email)
{
    return !user_repo_->existsByEmail(email);
}

int UserService::getUserCount()
{
    return user_repo_->getCount();
}

std::string UserService::getUserStatistics()
{
    int total_users = getUserCount();
    std::ostringstream oss;
    oss << "User Statistics:\n";
    oss << "Total users: " << total_users << "\n";
    
    return oss.str();
}

void UserService::setError(const std::string& error)
{
    last_error_ = error;
    Logger::error("UserService Error: {}", error);
}

std::string UserService::sanitizeInput(const std::string& input)
{
    std::string result = input;
    
    // Remove leading and trailing whitespace
    result.erase(0, result.find_first_not_of(" \t\n\r"));
    result.erase(result.find_last_not_of(" \t\n\r") + 1);
    
    // Remove potentially dangerous characters
    result.erase(std::remove(result.begin(), result.end(), '\''), result.end());
    result.erase(std::remove(result.begin(), result.end(), '"'), result.end());
    result.erase(std::remove(result.begin(), result.end(), ';'), result.end());
    result.erase(std::remove(result.begin(), result.end(), '\\'), result.end());
    
    return result;
}

void UserService::invalidateUserCache(int user_id)
{
    if (cache_service_)
    {
        cache_service_->invalidateUser(std::to_string(user_id));
    }
}

void UserService::invalidateUserCache(const std::string& username)
{
    if (cache_service_)
    {
        cache_service_->invalidateUser(username);
    }
}

void UserService::invalidateAllUserCache()
{
    if (cache_service_)
    {
        cache_service_->clearUserCache();
    }
}

std::string UserService::serializeUser(const User& user)
{
    Json::Value root;
    root["id"] = user.getId();
    root["username"] = user.getUsername();
    root["email"] = user.getEmail();
    root["created_at"] = user.getCreatedAt();
    return toJsonString(root);
}

std::optional<User> UserService::deserializeUser(const std::string& data)
{
    try
    {
        Json::CharReaderBuilder builder;
        Json::Value root;
        std::string errors;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(data.data(), data.data() + data.size(), &root, &errors))
        {
            Logger::error("Failed to parse user JSON: {}", errors);
            return std::nullopt;
        }

        return userFromJsonValue(root);
    }
    catch (const std::exception& e)
    {
        Logger::error("Failed to deserialize user: {}", e.what());
        return std::nullopt;
    }
}

std::string UserService::serializeUserList(const std::vector<User>& users)
{
    Json::Value root(Json::arrayValue);
    for (const auto& user : users)
    {
        Json::Value entry;
        entry["id"] = user.getId();
        entry["username"] = user.getUsername();
        entry["email"] = user.getEmail();
        entry["created_at"] = user.getCreatedAt();
        root.append(entry);
    }
    return toJsonString(root);
}

std::vector<User> UserService::deserializeUserList(const std::string& data)
{
    std::vector<User> users;
    try
    {
        Json::CharReaderBuilder builder;
        Json::Value root;
        std::string errors;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(data.data(), data.data() + data.size(), &root, &errors))
        {
            Logger::error("Failed to parse user list JSON: {}", errors);
            return users;
        }

        if (!root.isArray())
        {
            return users;
        }

        for (const auto& entry : root)
        {
            auto user = userFromJsonValue(entry);
            if (user.has_value())
            {
                users.push_back(user.value());
            }
        }
    }
    catch (const std::exception& e)
    {
        Logger::error("Failed to deserialize user list: {}", e.what());
    }
    return users;
}

std::string UserService::getUserCacheKey(int user_id)
{
    return "user:" + std::to_string(user_id);
}

std::string UserService::getUserCacheKey(const std::string& username)
{
    return "user:username:" + username;
}

std::string UserService::getUserListCacheKey(const std::string& list_type)
{
    return "userlist:" + list_type;
}

std::string UserService::getSearchCacheKey(const std::string& search_term)
{
    return "usersearch:" + search_term;
}
