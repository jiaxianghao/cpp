#include "DatabaseManager.h"
#include <iostream>

namespace db_util {

DatabaseManager::DatabaseManager()
    : driver_(nullptr)
    , connected_(false)
{
    try
    {
        driver_ = sql::mysql::get_mysql_driver_instance();
    }
    catch (sql::SQLException& e)
    {
        lastError_ = "Failed to get MySQL driver: " + std::string(e.what());
    }
}

DatabaseManager::~DatabaseManager()
{
    disconnect();
}

bool DatabaseManager::connect(const std::string& host, const std::string& user,
                             const std::string& password, const std::string& database,
                             int port)
{
    if (!driver_)
    {
        lastError_ = "MySQL driver not available";
        return false;
    }

    try
    {
        // Create connection URL
        std::string url = "tcp://" + host + ":" + std::to_string(port);
        
        // Create connection
        connection_.reset(driver_->connect(url, user, password));
        
        // Set database
        connection_->setSchema(database);
        
        // Set autocommit to true by default
        connection_->setAutoCommit(true);
        
        connected_ = true;
        lastError_.clear();
        return true;
    }
    catch (sql::SQLException& e)
    {
        lastError_ = "Connection failed: " + std::string(e.what());
        connected_ = false;
        return false;
    }
}

void DatabaseManager::disconnect()
{
    if (connection_)
    {
        connection_->close();
        connection_.reset();
    }
    connected_ = false;
}

bool DatabaseManager::isConnected() const
{
    return connected_ && connection_ && !connection_->isClosed();
}

bool DatabaseManager::executeQuery(const std::string& query)
{
    if (!isConnected())
    {
        lastError_ = "Not connected to database";
        return false;
    }

    try
    {
        std::unique_ptr<sql::Statement> stmt(connection_->createStatement());
        bool result = stmt->execute(query);
        
        // Consume any result set to avoid "Commands out of sync" error
        if (result)
        {
            std::unique_ptr<sql::ResultSet> rs(stmt->getResultSet());
            while (rs && rs->next())
            {
                // Consume the result set
            }
        }
        else
        {
            // For non-SELECT queries, check the update count
            int updateCount = stmt->getUpdateCount();
            if (updateCount >= 0)
            {
                // This is a successful non-SELECT query
                result = true;
            }
        }
        
        lastError_.clear();
        return result;
    }
    catch (sql::SQLException& e)
    {
        lastError_ = "Query execution failed: " + std::string(e.what()) + 
                     " (Error Code: " + std::to_string(e.getErrorCode()) + 
                     ", SQL State: " + e.getSQLState() + ")";
        std::cerr << "SQL Error: " << lastError_ << std::endl;
        return false;
    }
    catch (const std::exception& e)
    {
        lastError_ = "General exception: " + std::string(e.what());
        std::cerr << "Exception: " << lastError_ << std::endl;
        return false;
    }
}

std::unique_ptr<sql::ResultSet> DatabaseManager::executeSelect(const std::string& query)
{
    if (!isConnected())
    {
        lastError_ = "Not connected to database";
        return nullptr;
    }

    try
    {
        std::unique_ptr<sql::Statement> stmt(connection_->createStatement());
        std::unique_ptr<sql::ResultSet> result(stmt->executeQuery(query));
        lastError_.clear();
        return result;
    }
    catch (sql::SQLException& e)
    {
        lastError_ = "Select query failed: " + std::string(e.what());
        return nullptr;
    }
}

std::unique_ptr<sql::PreparedStatement> DatabaseManager::prepareStatement(const std::string& query)
{
    if (!isConnected())
    {
        lastError_ = "Not connected to database";
        return nullptr;
    }

    try
    {
        std::unique_ptr<sql::PreparedStatement> stmt(connection_->prepareStatement(query));
        lastError_.clear();
        return stmt;
    }
    catch (sql::SQLException& e)
    {
        lastError_ = "Failed to prepare statement: " + std::string(e.what());
        return nullptr;
    }
}

bool DatabaseManager::executePreparedStatement(sql::PreparedStatement* stmt)
{
    if (!stmt)
    {
        lastError_ = "Invalid prepared statement";
        return false;
    }

    try
    {
        bool result = stmt->execute();
        
        // For non-SELECT queries, if execute() returns false, it usually means success
        // for INSERT, UPDATE, DELETE operations
        if (!result)
        {
            // This is likely a successful non-SELECT query
            result = true;
        }
        
        lastError_.clear();
        return result;
    }
    catch (sql::SQLException& e)
    {
        lastError_ = "Prepared statement execution failed: " + std::string(e.what()) + 
                     " (Error Code: " + std::to_string(e.getErrorCode()) + 
                     ", SQL State: " + e.getSQLState() + ")";
        std::cerr << "SQL Error: " << lastError_ << std::endl;
        return false;
    }
    catch (const std::exception& e)
    {
        lastError_ = "General exception: " + std::string(e.what());
        std::cerr << "Exception: " << lastError_ << std::endl;
        return false;
    }
}

std::unique_ptr<sql::ResultSet> DatabaseManager::executePreparedSelect(sql::PreparedStatement* stmt)
{
    if (!stmt)
    {
        lastError_ = "Invalid prepared statement";
        return nullptr;
    }

    try
    {
        std::unique_ptr<sql::ResultSet> result(stmt->executeQuery());
        lastError_.clear();
        return result;
    }
    catch (sql::SQLException& e)
    {
        lastError_ = "Prepared select failed: " + std::string(e.what());
        return nullptr;
    }
}

bool DatabaseManager::beginTransaction()
{
    if (!isConnected())
    {
        lastError_ = "Not connected to database";
        return false;
    }

    try
    {
        connection_->setAutoCommit(false);
        lastError_.clear();
        return true;
    }
    catch (sql::SQLException& e)
    {
        lastError_ = "Failed to begin transaction: " + std::string(e.what());
        return false;
    }
}

bool DatabaseManager::commitTransaction()
{
    if (!isConnected())
    {
        lastError_ = "Not connected to database";
        return false;
    }

    try
    {
        connection_->commit();
        connection_->setAutoCommit(true);
        lastError_.clear();
        return true;
    }
    catch (sql::SQLException& e)
    {
        lastError_ = "Failed to commit transaction: " + std::string(e.what());
        return false;
    }
}

bool DatabaseManager::rollbackTransaction()
{
    if (!isConnected())
    {
        lastError_ = "Not connected to database";
        return false;
    }

    try
    {
        connection_->rollback();
        connection_->setAutoCommit(true);
        lastError_.clear();
        return true;
    }
    catch (sql::SQLException& e)
    {
        lastError_ = "Failed to rollback transaction: " + std::string(e.what());
        return false;
    }
}

std::string DatabaseManager::getLastError() const
{
    return lastError_;
}

int DatabaseManager::getLastInsertId() const
{
    if (!isConnected())
    {
        return -1;
    }

    try
    {
        std::unique_ptr<sql::Statement> stmt(connection_->createStatement());
        std::unique_ptr<sql::ResultSet> result(stmt->executeQuery("SELECT LAST_INSERT_ID()"));
        
        if (result->next())
        {
            return result->getInt(1);
        }
        return -1;
    }
    catch (sql::SQLException& e)
    {
        return -1;
    }
}

int DatabaseManager::getAffectedRows() const
{
    if (!isConnected())
    {
        return -1;
    }

    try
    {
        std::unique_ptr<sql::Statement> stmt(connection_->createStatement());
        return stmt->getUpdateCount();
    }
    catch (sql::SQLException& e)
    {
        return -1;
    }
}
} // namespace db_util
