#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

class ConfigManager
{
public:
    static ConfigManager& getInstance();
    
    // Delete copy and move
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ConfigManager(ConfigManager&&) = delete;
    ConfigManager& operator=(ConfigManager&&) = delete;
    
    // Load configuration from file
    bool loadConfig(const std::string& config_file);
    
    // Get configuration values
    std::string getString(const std::string& key, const std::string& default_value = "") const;
    int getInt(const std::string& key, int default_value = 0) const;
    bool getBool(const std::string& key, bool default_value = false) const;
    double getDouble(const std::string& key, double default_value = 0.0) const;
    
    // Set configuration values
    void setString(const std::string& key, const std::string& value);
    void setInt(const std::string& key, int value);
    void setBool(const std::string& key, bool value);
    void setDouble(const std::string& key, double value);
    
    // Check if key exists
    bool hasKey(const std::string& key) const;
    
    // Save configuration to file
    bool saveConfig(const std::string& config_file) const;
    
    // Get all configuration as string for debugging
    std::string toString() const;
    
    // Database specific configurations
    struct DatabaseConfig {
        std::string host = "localhost";
        int port = 3306;
        std::string user = "root";
        std::string password = "";
        std::string database = "myapp";
        int max_connections = 10;
        int min_connections = 5;
        int connection_timeout = 30;
        bool auto_reconnect = true;
    };
    
    struct CacheConfig {
        std::string host = "localhost";
        int port = 6379;
        int database = 0;
        std::string password = "";
        int connection_timeout = 5;
        int socket_timeout = 5;
        int pool_size = 10;
    };
    
    struct LogConfig {
        std::string level = "info";
        std::string file = "logs/app.log";
        size_t max_file_size = 5 * 1024 * 1024; // 5MB
        size_t max_files = 3;
        bool console_output = true;
        bool file_output = true;
    };
    
    struct ServerConfig {
        std::string host = "0.0.0.0";
        int port = 8080;
        int thread_pool_size = 4;
        int request_timeout = 30;
        bool enable_ssl = false;
        std::string ssl_cert_file = "";
        std::string ssl_key_file = "";
    };
    
    // Get specific configuration sections
    DatabaseConfig getDatabaseConfig() const;
    CacheConfig getCacheConfig() const;
    LogConfig getLogConfig() const;
    ServerConfig getServerConfig() const;
    
private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::string> config_values_;
    
    // Parse JSON file
    bool parseJsonFile(const std::string& filename);
    
    // Convert JSON to config map
    void jsonToConfig(const std::string& json_content);
    
    // Convert config map to JSON
    std::string configToJson() const;
    
    // Helper functions for type conversion
    std::string toLower(const std::string& str) const;
    bool isTrue(const std::string& value) const;
};

#endif // CONFIG_MANAGER_H