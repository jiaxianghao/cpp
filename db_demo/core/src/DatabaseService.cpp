#include "DatabaseService.h"
#include <iostream>

namespace core {

DatabaseService& DatabaseService::getInstance()
{
    static DatabaseService instance;
    return instance;
}

bool DatabaseService::initialize(const std::string& host,
                                const std::string& user,
                                const std::string& password,
                                const std::string& database,
                                int port)
{
    if (initialized_)
    {
        std::cout << "DatabaseService already initialized" << std::endl;
        return true;
    }

    // Connect to database
    if (!dbManager_.connect(host, user, password, database, port))
    {
        std::cerr << "Failed to connect to database: " << dbManager_.getLastError() << std::endl;
        return false;
    }

    // Initialize TableManager
    tableManager_ = std::make_unique<db_util::TableManager>(dbManager_);

    // Initialize all DAOs
    userDAO_ = std::make_unique<dao::UserDAO>(*tableManager_);
    orderDAO_ = std::make_unique<dao::OrderDAO>(*tableManager_);
    productDAO_ = std::make_unique<dao::ProductDAO>(*tableManager_);

    initialized_ = true;
    std::cout << "DatabaseService initialized successfully" << std::endl;
    return true;
}

dao::UserDAO& DatabaseService::getUserDAO()
{
    if (!initialized_)
    {
        throw std::runtime_error("DatabaseService not initialized");
    }
    return *userDAO_;
}

dao::OrderDAO& DatabaseService::getOrderDAO()
{
    if (!initialized_)
    {
        throw std::runtime_error("DatabaseService not initialized");
    }
    return *orderDAO_;
}

dao::ProductDAO& DatabaseService::getProductDAO()
{
    if (!initialized_)
    {
        throw std::runtime_error("DatabaseService not initialized");
    }
    return *productDAO_;
}

bool DatabaseService::beginTransaction()
{
    if (!initialized_)
    {
        return false;
    }
    return tableManager_->beginTransaction();
}

bool DatabaseService::commitTransaction()
{
    if (!initialized_)
    {
        return false;
    }
    return tableManager_->commitTransaction();
}

bool DatabaseService::rollbackTransaction()
{
    if (!initialized_)
    {
        return false;
    }
    return tableManager_->rollbackTransaction();
}

db_util::DatabaseManager& DatabaseService::getDatabaseManager()
{
    if (!initialized_)
    {
        throw std::runtime_error("DatabaseService not initialized");
    }
    return dbManager_;
}

db_util::TableManager& DatabaseService::getTableManager()
{
    if (!initialized_)
    {
        throw std::runtime_error("DatabaseService not initialized");
    }
    return *tableManager_;
}

bool DatabaseService::initializeTables()
{
    if (!initialized_)
    {
        std::cerr << "DatabaseService not initialized" << std::endl;
        return false;
    }

    // Create users table
    std::map<std::string, std::string> userFields =
    {
        {"id", "INT AUTO_INCREMENT PRIMARY KEY"},
        {"name", "VARCHAR(100) NOT NULL"},
        {"email", "VARCHAR(100) UNIQUE NOT NULL"},
        {"age", "INT"},
        {"is_active", "BOOLEAN DEFAULT TRUE"},
        {"created_at", "TIMESTAMP DEFAULT CURRENT_TIMESTAMP"}
    };

    if (!tableManager_->createTable("users", userFields))
    {
        std::cerr << "Failed to create users table: " << tableManager_->getLastError() << std::endl;
        return false;
    }

    // Create orders table
    std::map<std::string, std::string> orderFields =
    {
        {"id", "INT AUTO_INCREMENT PRIMARY KEY"},
        {"user_id", "INT NOT NULL"},
        {"product_name", "VARCHAR(200) NOT NULL"},
        {"quantity", "INT NOT NULL"},
        {"total_price", "DECIMAL(10, 2) NOT NULL"},
        {"status", "VARCHAR(50) DEFAULT 'pending'"},
        {"created_at", "TIMESTAMP DEFAULT CURRENT_TIMESTAMP"},
        {"FOREIGN KEY (user_id)", "REFERENCES users(id)"}
    };

    if (!tableManager_->createTable("orders", orderFields))
    {
        std::cerr << "Failed to create orders table: " << tableManager_->getLastError() << std::endl;
        return false;
    }

    // Create products table
    std::map<std::string, std::string> productFields =
    {
        {"id", "INT AUTO_INCREMENT PRIMARY KEY"},
        {"name", "VARCHAR(200) NOT NULL"},
        {"description", "TEXT"},
        {"price", "DECIMAL(10, 2) NOT NULL"},
        {"stock", "INT DEFAULT 0"},
        {"available", "BOOLEAN DEFAULT TRUE"},
        {"created_at", "TIMESTAMP DEFAULT CURRENT_TIMESTAMP"}
    };

    if (!tableManager_->createTable("products", productFields))
    {
        std::cerr << "Failed to create products table: " << tableManager_->getLastError() << std::endl;
        return false;
    }

    std::cout << "All tables initialized successfully" << std::endl;
    return true;
}

} // namespace core
