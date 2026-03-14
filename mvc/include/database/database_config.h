#ifndef DATABASE_CONFIG_H
#define DATABASE_CONFIG_H

#include <string>
#include <memory>
#include "utils/config_manager.h"

class DatabaseConfig
{
public:
    static DatabaseConfig& getInstance();
    
    // Initialize from configuration file or use defaults
    void initialize(const std::string& config_file = "");
    
    // Getters
    const std::string& getHost() const { return host_; }
    const std::string& getUser() const { return user_; }
    const std::string& getPassword() const { return password_; }
    const std::string& getDatabase() const { return database_; }
    int getPort() const { return port_; }
    int getMaxConnections() const { return max_connections_; }
    int getMinConnections() const { return min_connections_; }
    int getConnectionTimeout() const { return connection_timeout_; }
    bool getAutoReconnect() const { return auto_reconnect_; }
    
    // Setters
    void setHost(const std::string& host) { host_ = host; }
    void setUser(const std::string& user) { user_ = user; }
    void setPassword(const std::string& password) { password_ = password; }
    void setDatabase(const std::string& database) { database_ = database; }
    void setPort(int port) { port_ = port; }
    void setMaxConnections(int max_connections) { max_connections_ = max_connections; }
    void setMinConnections(int min_connections) { min_connections_ = min_connections; }
    void setConnectionTimeout(int timeout) { connection_timeout_ = timeout; }
    void setAutoReconnect(bool auto_reconnect) { auto_reconnect_ = auto_reconnect; }

private:
    DatabaseConfig() = default;
    DatabaseConfig(const DatabaseConfig&) = delete;
    DatabaseConfig& operator=(const DatabaseConfig&) = delete;
    
    std::string host_ = "localhost";
    std::string user_ = "root";
    std::string password_ = "password";
    std::string database_ = "myapp";
    int port_ = 3306;
    int max_connections_ = 10;
    int min_connections_ = 5;
    int connection_timeout_ = 30;
    bool auto_reconnect_ = true;
};

#endif // DATABASE_CONFIG_H
