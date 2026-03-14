#ifndef USER_H
#define USER_H

#include <string>
#include <ctime>

class User
{
public:
    User() = default;
    User(int id, const std::string& username, const std::string& email);
    User(const std::string& username, const std::string& email);
    
    // Getters
    int getId() const { return id_; }
    const std::string& getUsername() const { return username_; }
    const std::string& getEmail() const { return email_; }
    const std::string& getCreatedAt() const { return created_at_; }
    
    // Setters
    void setId(int id) { id_ = id; }
    void setUsername(const std::string& username) { username_ = username; }
    void setEmail(const std::string& email) { email_ = email; }
    void setCreatedAt(const std::string& created_at) { created_at_ = created_at; }
    
    // Utility methods
    bool isValid() const;
    std::string toString() const;

private:
    int id_ = -1;
    std::string username_;
    std::string email_;
    std::string created_at_;
};

#endif // USER_H
