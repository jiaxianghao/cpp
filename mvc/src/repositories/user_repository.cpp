#include "repositories/user_repository.h"
#include <iostream>

using Param = DatabaseConnection::PreparedParam;

UserRepository::UserRepository(std::shared_ptr<DatabaseConnection> connection)
    : connection_(connection)
{
}

std::vector<User> UserRepository::getAll()
{
    std::vector<User> users;
    std::string query = "SELECT id, username, email, created_at FROM users ORDER BY id";
    
    auto results = connection_->fetchAll(query);
    for (const auto& row : results)
    {
        users.push_back(mapRowToUser(row));
    }
    
    return users;
}

User UserRepository::getById(int id)
{
    auto result = connection_->fetchOnePrepared(
        "SELECT id, username, email, created_at FROM users WHERE id = ?",
        { Param::fromInt(id) });
    
    if (result.empty())
    {
        return User(); // Return empty user if not found
    }
    
    return mapRowToUser(result);
}

User UserRepository::getByUsername(const std::string& username)
{
    auto result = connection_->fetchOnePrepared(
        "SELECT id, username, email, created_at FROM users WHERE username = ?",
        { Param::fromString(username) });
    
    if (result.empty())
    {
        return User();
    }
    
    return mapRowToUser(result);
}

User UserRepository::getByEmail(const std::string& email)
{
    auto result = connection_->fetchOnePrepared(
        "SELECT id, username, email, created_at FROM users WHERE email = ?",
        { Param::fromString(email) });
    
    if (result.empty())
    {
        return User();
    }
    
    return mapRowToUser(result);
}

int UserRepository::create(const User& user)
{
    if (connection_->executePreparedQuery(
        "INSERT INTO users (username, email) VALUES (?, ?)",
        { Param::fromString(user.getUsername()), Param::fromString(user.getEmail()) }))
    {
        // Get the last inserted ID
        auto result = connection_->fetchOne("SELECT LAST_INSERT_ID() as id");
        if (!result.empty())
        {
            return std::stoi(result["id"]);
        }
    }
    
    return -1; // Return -1 on failure
}

bool UserRepository::update(const User& user)
{
    return connection_->executePreparedQuery(
        "UPDATE users SET username = ?, email = ? WHERE id = ?",
        { Param::fromString(user.getUsername()), Param::fromString(user.getEmail()),
            Param::fromInt(user.getId()) });
}

bool UserRepository::deleteById(int id)
{
    return connection_->executePreparedQuery(
        "DELETE FROM users WHERE id = ?",
        { Param::fromInt(id) });
}

std::vector<User> UserRepository::searchByUsername(const std::string& pattern)
{
    std::vector<User> users;
    std::string like_pattern = "%" + pattern + "%";
    auto results = connection_->fetchAllPrepared(
        "SELECT id, username, email, created_at FROM users WHERE username LIKE ? ORDER BY username",
        { Param::fromString(like_pattern) });
    for (const auto& row : results)
    {
        users.push_back(mapRowToUser(row));
    }
    
    return users;
}

std::vector<User> UserRepository::searchByEmail(const std::string& pattern)
{
    std::vector<User> users;
    std::string like_pattern = "%" + pattern + "%";
    auto results = connection_->fetchAllPrepared(
        "SELECT id, username, email, created_at FROM users WHERE email LIKE ? ORDER BY email",
        { Param::fromString(like_pattern) });
    for (const auto& row : results)
    {
        users.push_back(mapRowToUser(row));
    }
    
    return users;
}

int UserRepository::getCount()
{
    auto result = connection_->fetchOne("SELECT COUNT(*) as count FROM users");
    if (!result.empty())
    {
        return std::stoi(result["count"]);
    }
    return 0;
}

bool UserRepository::exists(int id)
{
    auto result = connection_->fetchOnePrepared(
        "SELECT 1 FROM users WHERE id = ?",
        { Param::fromInt(id) });
    return !result.empty();
}

bool UserRepository::existsByUsername(const std::string& username)
{
    auto result = connection_->fetchOnePrepared(
        "SELECT 1 FROM users WHERE username = ?",
        { Param::fromString(username) });
    return !result.empty();
}

bool UserRepository::existsByEmail(const std::string& email)
{
    auto result = connection_->fetchOnePrepared(
        "SELECT 1 FROM users WHERE email = ?",
        { Param::fromString(email) });
    return !result.empty();
}

User UserRepository::mapRowToUser(const std::map<std::string, std::string>& row)
{
    User user;
    
    if (row.find("id") != row.end())
    {
        user.setId(std::stoi(row.at("id")));
    }
    
    if (row.find("username") != row.end())
    {
        user.setUsername(row.at("username"));
    }
    
    if (row.find("email") != row.end())
    {
        user.setEmail(row.at("email"));
    }
    
    if (row.find("created_at") != row.end())
    {
        user.setCreatedAt(row.at("created_at"));
    }
    
    return user;
}
