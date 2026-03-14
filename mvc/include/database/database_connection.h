#ifndef DATABASE_CONNECTION_H
#define DATABASE_CONNECTION_H

#include <mysql/mysql.h>
#include <memory>
#include <string>
#include <vector>
#include <map>

class DatabaseConnection
{
public:
    struct PreparedParam
    {
        enum class Type
        {
            Int,
            Double,
            String
        };

        Type type;
        int int_value = 0;
        double double_value = 0.0;
        std::string string_value;

        static PreparedParam fromInt(int value);
        static PreparedParam fromDouble(double value);
        static PreparedParam fromString(std::string value);
    };

    explicit DatabaseConnection();
    ~DatabaseConnection();
    
    // Disable copy constructor and assignment operator
    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;
    
    // Connection management
    bool connect();
    void disconnect();
    bool isConnected() const { return connected_; }
    
    // Query execution
    bool executeQuery(const std::string& query);
    std::vector<std::map<std::string, std::string>> fetchAll(const std::string& query);
    std::map<std::string, std::string> fetchOne(const std::string& query);

    bool executePreparedQuery(const std::string& query, const std::vector<PreparedParam>& params);
    std::vector<std::map<std::string, std::string>> fetchAllPrepared(const std::string& query,
        const std::vector<PreparedParam>& params);
    std::map<std::string, std::string> fetchOnePrepared(const std::string& query,
        const std::vector<PreparedParam>& params);
    
    // Prepared statements
    bool prepareStatement(const std::string& query);
    bool bindParameter(int index, const std::string& value);
    bool bindParameter(int index, int value);
    std::vector<std::map<std::string, std::string>> executePrepared();
    
    // Transaction management
    bool beginTransaction();
    bool commit();
    bool rollback();
    
    // Error handling
    std::string getLastError() const { return last_error_; }

private:
    MYSQL* mysql_;
    MYSQL_STMT* stmt_;
    bool connected_;
    std::string last_error_;
    
    void setError(const std::string& error);
};

#endif // DATABASE_CONNECTION_H
