#ifndef USER_REPOSITORY_H
#define USER_REPOSITORY_H

#include "models/user.h"
#include "database/database_connection.h"
#include <vector>
#include <memory>

class UserRepository
{
public:
    explicit UserRepository(std::shared_ptr<DatabaseConnection> connection);
    
    // CRUD operations
    std::vector<User> getAll();
    User getById(int id);
    User getByUsername(const std::string& username);
    User getByEmail(const std::string& email);
    
    int create(const User& user);
    bool update(const User& user);
    bool deleteById(int id);
    
    // Search operations
    std::vector<User> searchByUsername(const std::string& pattern);
    std::vector<User> searchByEmail(const std::string& pattern);
    
    // Statistics
    int getCount();
    bool exists(int id);
    bool existsByUsername(const std::string& username);
    bool existsByEmail(const std::string& email);

private:
    std::shared_ptr<DatabaseConnection> connection_;
    
    User mapRowToUser(const std::map<std::string, std::string>& row);
};

#endif // USER_REPOSITORY_H
