#include "database/database_config.h"
#include "utils/logger.h"

DatabaseConfig& DatabaseConfig::getInstance()
{
    static DatabaseConfig instance;
    return instance;
}

void DatabaseConfig::initialize(const std::string& config_file)
{
    if (config_file.empty())
    {
        Logger::info("Using default database configuration");
        return;
    }
    
    auto& config_manager = ConfigManager::getInstance();
    if (!config_manager.loadConfig(config_file))
    {
        Logger::warn("Failed to load configuration file: {}, using defaults", config_file);
        return;
    }
    
    auto db_config = config_manager.getDatabaseConfig();
    
    host_ = db_config.host;
    port_ = db_config.port;
    user_ = db_config.user;
    password_ = db_config.password;
    database_ = db_config.database;
    max_connections_ = db_config.max_connections;
    min_connections_ = db_config.min_connections;
    connection_timeout_ = db_config.connection_timeout;
    auto_reconnect_ = db_config.auto_reconnect;
    
    Logger::info("Database configuration loaded from file: {}", config_file);
    Logger::info("Database: {}@{}:{}/{}", user_, host_, port_, database_);
}
