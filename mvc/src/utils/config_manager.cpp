#include "utils/config_manager.h"
#include "utils/logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>

// Simple JSON parser implementation for basic needs
// In production, consider using a proper JSON library like nlohmann/json

bool ConfigManager::loadConfig(const std::string& config_file)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    Logger::info("Loading configuration from file: {}", config_file);
    
    if (!parseJsonFile(config_file))
    {
        Logger::error("Failed to parse configuration file: {}", config_file);
        return false;
    }
    
    Logger::info("Configuration loaded successfully");
    return true;
}

bool ConfigManager::parseJsonFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        Logger::error("Cannot open configuration file: {}", filename);
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json_content = buffer.str();
    file.close();
    
    jsonToConfig(json_content);
    return true;
}

void ConfigManager::jsonToConfig(const std::string& json_content)
{
    // Simple JSON parsing - remove braces and quotes, split by commas
    std::string content = json_content;
    
    // Remove whitespace
    content.erase(std::remove_if(content.begin(), content.end(), ::isspace), content.end());
    
    // Remove outer braces
    if (content.front() == '{' && content.back() == '}')
    {
        content = content.substr(1, content.length() - 2);
    }
    
    // Split by sections (database, cache, log, server)
    size_t pos = 0;
    while (pos < content.length())
    {
        size_t section_start = content.find('"', pos);
        if (section_start == std::string::npos) break;
        
        size_t section_end = content.find('"', section_start + 1);
        if (section_end == std::string::npos) break;
        
        std::string section_name = content.substr(section_start + 1, section_end - section_start - 1);
        
        size_t brace_start = content.find('{', section_end);
        if (brace_start == std::string::npos) break;
        
        size_t brace_end = brace_start;
        int brace_count = 1;
        while (brace_end < content.length() && brace_count > 0)
        {
            brace_end++;
            if (content[brace_end] == '{') brace_count++;
            else if (content[brace_end] == '}') brace_count--;
        }
        
        if (brace_count == 0)
        {
            std::string section_content = content.substr(brace_start + 1, brace_end - brace_start - 1);
            
            // Parse key-value pairs within section
            size_t kv_pos = 0;
            while (kv_pos < section_content.length())
            {
                size_t key_start = section_content.find('"', kv_pos);
                if (key_start == std::string::npos) break;
                
                size_t key_end = section_content.find('"', key_start + 1);
                if (key_end == std::string::npos) break;
                
                std::string key = section_content.substr(key_start + 1, key_end - key_start - 1);
                
                size_t colon_pos = section_content.find(':', key_end);
                if (colon_pos == std::string::npos) break;
                
                size_t value_start = colon_pos + 1;
                size_t value_end = section_content.find(',', value_start);
                if (value_end == std::string::npos) value_end = section_content.length();
                
                std::string value = section_content.substr(value_start, value_end - value_start);
                
                // Remove quotes from value if present
                if (value.front() == '"' && value.back() == '"')
                {
                    value = value.substr(1, value.length() - 2);
                }
                
                // Store with section prefix
                std::string full_key = section_name + "." + key;
                config_values_[full_key] = value;
                
                kv_pos = value_end + 1;
            }
        }
        
        pos = brace_end + 1;
    }
}

std::string ConfigManager::getString(const std::string& key, const std::string& default_value) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = config_values_.find(key);
    if (it != config_values_.end())
    {
        return it->second;
    }
    return default_value;
}

int ConfigManager::getInt(const std::string& key, int default_value) const
{
    std::string value = getString(key, "");
    if (value.empty()) return default_value;
    
    try
    {
        return std::stoi(value);
    }
    catch (const std::exception& e)
    {
        Logger::warn("Failed to convert '{}' to int for key '{}', using default: {}", 
                    value, key, default_value);
        return default_value;
    }
}

bool ConfigManager::getBool(const std::string& key, bool default_value) const
{
    std::string value = getString(key, "");
    if (value.empty()) return default_value;
    
    return isTrue(value);
}

double ConfigManager::getDouble(const std::string& key, double default_value) const
{
    std::string value = getString(key, "");
    if (value.empty()) return default_value;
    
    try
    {
        return std::stod(value);
    }
    catch (const std::exception& e)
    {
        Logger::warn("Failed to convert '{}' to double for key '{}', using default: {}", 
                    value, key, default_value);
        return default_value;
    }
}

void ConfigManager::setString(const std::string& key, const std::string& value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    config_values_[key] = value;
}

void ConfigManager::setInt(const std::string& key, int value)
{
    setString(key, std::to_string(value));
}

void ConfigManager::setBool(const std::string& key, bool value)
{
    setString(key, value ? "true" : "false");
}

void ConfigManager::setDouble(const std::string& key, double value)
{
    setString(key, std::to_string(value));
}

bool ConfigManager::hasKey(const std::string& key) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return config_values_.find(key) != config_values_.end();
}

bool ConfigManager::saveConfig(const std::string& config_file) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string json_content = configToJson();
    
    std::ofstream file(config_file);
    if (!file.is_open())
    {
        Logger::error("Cannot open configuration file for writing: {}", config_file);
        return false;
    }
    
    file << json_content;
    file.close();
    
    Logger::info("Configuration saved to file: {}", config_file);
    return true;
}

std::string ConfigManager::configToJson() const
{
    std::ostringstream json;
    json << "{\n";
    
    // Group by sections
    std::map<std::string, std::map<std::string, std::string>> sections;
    
    for (const auto& pair : config_values_)
    {
        size_t dot_pos = pair.first.find('.');
        if (dot_pos != std::string::npos)
        {
            std::string section = pair.first.substr(0, dot_pos);
            std::string key = pair.first.substr(dot_pos + 1);
            sections[section][key] = pair.second;
        }
    }
    
    bool first_section = true;
    for (const auto& section : sections)
    {
        if (!first_section) json << ",\n";
        json << "  \"" << section.first << "\": {\n";
        
        bool first_key = true;
        for (const auto& key_value : section.second)
        {
            if (!first_key) json << ",\n";
            
            // Check if value is numeric or boolean
            std::string value = key_value.second;
            bool is_numeric = false;
            bool is_bool = false;
            
            try
            {
                std::stod(value);
                is_numeric = true;
            }
            catch (...) {}
            
            if (value == "true" || value == "false")
            {
                is_bool = true;
            }
            
            json << "    \"" << key_value.first << "\": ";
            
            if (is_numeric || is_bool)
            {
                json << value;
            }
            else
            {
                json << "\"" << value << "\"";
            }
            
            first_key = false;
        }
        
        json << "\n  }";
        first_section = false;
    }
    
    json << "\n}";
    return json.str();
}

std::string ConfigManager::toString() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return configToJson();
}

std::string ConfigManager::toLower(const std::string& str) const
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

bool ConfigManager::isTrue(const std::string& value) const
{
    std::string lower_value = toLower(value);
    return lower_value == "true" || lower_value == "1" || lower_value == "yes" || lower_value == "on";
}

ConfigManager::DatabaseConfig ConfigManager::getDatabaseConfig() const
{
    DatabaseConfig config;
    config.host = getString("database.host", "localhost");
    config.port = getInt("database.port", 3306);
    config.user = getString("database.user", "root");
    config.password = getString("database.password", "");
    config.database = getString("database.database", "myapp");
    config.max_connections = getInt("database.max_connections", 10);
    config.min_connections = getInt("database.min_connections", 5);
    config.connection_timeout = getInt("database.connection_timeout", 30);
    config.auto_reconnect = getBool("database.auto_reconnect", true);
    return config;
}

ConfigManager::CacheConfig ConfigManager::getCacheConfig() const
{
    CacheConfig config;
    config.host = getString("cache.host", "localhost");
    config.port = getInt("cache.port", 6379);
    config.database = getInt("cache.database", 0);
    config.password = getString("cache.password", "");
    config.connection_timeout = getInt("cache.connection_timeout", 5);
    config.socket_timeout = getInt("cache.socket_timeout", 5);
    config.pool_size = getInt("cache.pool_size", 10);
    return config;
}

ConfigManager::LogConfig ConfigManager::getLogConfig() const
{
    LogConfig config;
    config.level = getString("log.level", "info");
    config.file = getString("log.file", "logs/app.log");
    config.max_file_size = getInt("log.max_file_size", 5 * 1024 * 1024);
    config.max_files = getInt("log.max_files", 3);
    config.console_output = getBool("log.console_output", true);
    config.file_output = getBool("log.file_output", true);
    return config;
}

ConfigManager::ServerConfig ConfigManager::getServerConfig() const
{
    ServerConfig config;
    config.host = getString("server.host", "0.0.0.0");
    config.port = getInt("server.port", 8080);
    config.thread_pool_size = getInt("server.thread_pool_size", 4);
    config.request_timeout = getInt("server.request_timeout", 30);
    config.enable_ssl = getBool("server.enable_ssl", false);
    config.ssl_cert_file = getString("server.ssl_cert_file", "");
    config.ssl_key_file = getString("server.ssl_key_file", "");
    return config;
}