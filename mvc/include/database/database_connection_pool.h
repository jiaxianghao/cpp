#ifndef DATABASE_CONNECTION_POOL_H
#define DATABASE_CONNECTION_POOL_H

#include "database_connection.h"
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>

// Connection wrapper that manages connection lifecycle
class PooledConnection
{
public:
    PooledConnection(std::shared_ptr<DatabaseConnection> conn,
                    std::function<void(std::shared_ptr<DatabaseConnection>)> return_func)
        : connection_(conn), return_func_(return_func)
    {
    }

    ~PooledConnection()
    {
        if (connection_ && return_func_)
        {
            return_func_(connection_);
        }
    }

    // Delete copy constructor and assignment operator
    PooledConnection(const PooledConnection&) = delete;
    PooledConnection& operator=(const PooledConnection&) = delete;

    // Move constructor and assignment
    PooledConnection(PooledConnection&& other) noexcept
        : connection_(std::move(other.connection_))
        , return_func_(std::move(other.return_func_))
    {
        other.return_func_ = nullptr;
    }

    PooledConnection& operator=(PooledConnection&& other) noexcept
    {
        if (this != &other)
        {
            connection_ = std::move(other.connection_);
            return_func_ = std::move(other.return_func_);
            other.return_func_ = nullptr;
        }
        return *this;
    }

    std::shared_ptr<DatabaseConnection> get() const { return connection_; }
    DatabaseConnection* operator->() const { return connection_.get(); }
    DatabaseConnection& operator*() const { return *connection_; }

private:
    std::shared_ptr<DatabaseConnection> connection_;
    std::function<void(std::shared_ptr<DatabaseConnection>)> return_func_;
};

// Database connection pool for managing multiple connections
class DatabaseConnectionPool
{
public:
    // Get singleton instance
    static DatabaseConnectionPool& getInstance();

    // Delete copy and move
    DatabaseConnectionPool(const DatabaseConnectionPool&) = delete;
    DatabaseConnectionPool& operator=(const DatabaseConnectionPool&) = delete;
    DatabaseConnectionPool(DatabaseConnectionPool&&) = delete;
    DatabaseConnectionPool& operator=(DatabaseConnectionPool&&) = delete;

    // Initialize the pool with specified parameters
    bool initialize(size_t min_connections = 5,
                   size_t max_connections = 20,
                   std::chrono::seconds timeout = std::chrono::seconds(30));

    // Get a connection from the pool
    PooledConnection getConnection();

    // Get pool statistics
    size_t getAvailableConnections() const;
    size_t getTotalConnections() const;
    size_t getActiveConnections() const;

    // Shutdown the pool
    void shutdown();

private:
    DatabaseConnectionPool();
    ~DatabaseConnectionPool();

    // Create a new database connection
    std::shared_ptr<DatabaseConnection> createConnection();

    // Return a connection to the pool
    void returnConnection(std::shared_ptr<DatabaseConnection> conn);

    // Check if connection is still valid
    bool isConnectionValid(std::shared_ptr<DatabaseConnection> conn);

    // Remove invalid connections
    void cleanupInvalidConnections();

    std::queue<std::shared_ptr<DatabaseConnection>> available_connections_;
    size_t total_connections_;
    size_t min_connections_;
    size_t max_connections_;
    std::chrono::seconds connection_timeout_;

    mutable std::mutex mutex_;
    std::condition_variable condition_;
    bool is_shutdown_;
};

#endif // DATABASE_CONNECTION_POOL_H
