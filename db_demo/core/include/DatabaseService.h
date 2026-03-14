#ifndef CORE_DATABASE_SERVICE_H
#define CORE_DATABASE_SERVICE_H

/**
 * @file DatabaseService.h
 * @brief Singleton database service for managing database connections and DAOs
 * @version 1.0.0
 */

#include "DatabaseManager.h"
#include "TableManager.h"
#include "dao/UserDAO.h"
#include "dao/OrderDAO.h"
#include "dao/ProductDAO.h"
#include <memory>
#include <string>

namespace core {

/**
 * @brief Singleton database service
 * 
 * Manages database connection and provides access to all DAOs
 * This is the main entry point for all database operations in the application
 */
class DatabaseService
{
public:
    /**
     * @brief Get singleton instance
     * @return Reference to DatabaseService instance
     */
    static DatabaseService& getInstance();

    /**
     * @brief Initialize database connection
     * @param host Database host
     * @param user Database user
     * @param password Database password
     * @param database Database name
     * @param port Database port (default: 3306)
     * @return true if successful, false otherwise
     */
    bool initialize(const std::string& host,
                   const std::string& user,
                   const std::string& password,
                   const std::string& database,
                   int port = 3306);

    /**
     * @brief Check if database service is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const
    {
        return initialized_;
    }

    /**
     * @brief Get UserDAO instance
     * @return Reference to UserDAO
     */
    dao::UserDAO& getUserDAO();

    /**
     * @brief Get OrderDAO instance
     * @return Reference to OrderDAO
     */
    dao::OrderDAO& getOrderDAO();

    /**
     * @brief Get ProductDAO instance
     * @return Reference to ProductDAO
     */
    dao::ProductDAO& getProductDAO();

    /**
     * @brief Begin transaction
     * @return true if successful, false otherwise
     */
    bool beginTransaction();

    /**
     * @brief Commit transaction
     * @return true if successful, false otherwise
     */
    bool commitTransaction();

    /**
     * @brief Rollback transaction
     * @return true if successful, false otherwise
     */
    bool rollbackTransaction();

    /**
     * @brief Get direct access to DatabaseManager (for special cases)
     * @return Reference to DatabaseManager
     */
    db_util::DatabaseManager& getDatabaseManager();

    /**
     * @brief Get direct access to TableManager (for special cases)
     * @return Reference to TableManager
     */
    db_util::TableManager& getTableManager();

    /**
     * @brief Initialize database tables
     * Creates all required tables if they don't exist
     * @return true if successful, false otherwise
     */
    bool initializeTables();

private:
    DatabaseService() = default;
    ~DatabaseService() = default;

    // Prevent copy
    DatabaseService(const DatabaseService&) = delete;
    DatabaseService& operator=(const DatabaseService&) = delete;

    bool initialized_ = false;

    // Core database components
    db_util::DatabaseManager dbManager_;
    std::unique_ptr<db_util::TableManager> tableManager_;

    // DAOs
    std::unique_ptr<dao::UserDAO> userDAO_;
    std::unique_ptr<dao::OrderDAO> orderDAO_;
    std::unique_ptr<dao::ProductDAO> productDAO_;
};

} // namespace core

#endif // CORE_DATABASE_SERVICE_H
