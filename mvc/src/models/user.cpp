#include "models/user.h"
#include <sstream>
#include <regex>

User::User(int id, const std::string& username, const std::string& email)
    : id_(id), username_(username), email_(email)
{
}

User::User(const std::string& username, const std::string& email)
    : username_(username), email_(email)
{
}

bool User::isValid() const
{
    if (username_.empty() || email_.empty())
    {
        return false;
    }
    
    // Simple email validation
    std::regex email_regex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return std::regex_match(email_, email_regex);
}

std::string User::toString() const
{
    std::ostringstream oss;
    oss << "User{id=" << id_ 
        << ", username='" << username_ 
        << "', email='" << email_ 
        << "', created_at='" << created_at_ << "'}";
    return oss.str();
}
