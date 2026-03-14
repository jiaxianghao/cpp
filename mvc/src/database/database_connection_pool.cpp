#include "database/database_connection_pool.h"
#include "utils/logger.h"
#include <stdexcept>

DatabaseConnectionPool::DatabaseConnectionPool()
    : total_connections_(0)
    , min_connections_(5)
    , max_connections_(20)
    , connection_timeout_(std::chrono::seconds(30))
    , is_shutdown_(false)
{
}

DatabaseConnectionPool::~DatabaseConnectionPool()
{
    shutdown();
}

DatabaseConnectionPool& DatabaseConnectionPool::getInstance()
{
    static DatabaseConnectionPool instance;
    return instance;
}

bool DatabaseConnectionPool::initialize(size_t min_connections,
                                       size_t max_connections,
                                       std::chrono::seconds timeout)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (total_connections_ > 0)
    {
        Logger::warn("Connection pool already initialized");
        return false;
    }

    // If parameters are not explicitly provided, try to load from config
    if (min_connections == 0 && max_connections == 0)
    {
        auto& config = DatabaseConfig::getInstance();
        min_connections_ = config.getMinConnections();
        max_connections_ = config.getMaxConnections();
        connection_timeout_ = std::chrono::seconds(config.getConnectionTimeout());
    }
    else
    {
        min_connections_ = min_connections;
        max_connections_ = max_connections;
        connection_timeout_ = timeout;
    }

    Logger::info("Initializing connection pool: min={}, max={}, timeout={}s",
                min_connections_, max_connections_, connection_timeout_.count());

    // Create minimum number of connections
    for (size_t i = 0; i < min_connections_; ++i)
    {
        auto conn = createConnection();
        if (conn && conn->isConnected())
        {
            available_connections_.push(conn);
            total_connections_++;
        }
        else
        {
            Logger::error("Failed to create initial connection {}/{}", i + 1, min_connections_);
            return false;
        }
    }

    Logger::info("Connection pool initialized successfully with {} connections", total_connections_);
    return true;
}

std::shared_ptr<DatabaseConnection> DatabaseConnectionPool::createConnection()
{
    auto conn = std::make_shared<DatabaseConnection>();
    if (!conn->connect())
    {
        Logger::error("Failed to create database connection: {}", conn->getLastError());
        return nullptr;
    }
    return conn;
}

PooledConnection DatabaseConnectionPool::getConnection()
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (is_shutdown_)
    {
        throw std::runtime_error("Connection pool is shut down");
    }

    // Wait for available connection or timeout
    auto wait_start = std::chrono::steady_clock::now();

    while (available_connections_.empty())
    {
        // Try to create a new connection if we haven't reached max
        if (total_connections_ < max_connections_)
        {
            lock.unlock();
            auto conn = createConnection();
            lock.lock();

            if (conn && conn->isConnected())
            {
                total_connections_++;
                Logger::debug("Created new connection, total connections: {}", total_connections_);
                return PooledConnection(conn, [this](auto c)
                {
                    this->returnConnection(c);
                });
            }
        }

        // Wait for a connection to become available
        auto elapsed = std::chrono::steady_clock::now() - wait_start;
        auto remaining = connection_timeout_ - std::chrono::duration_cast<std::chrono::seconds>(elapsed);

        if (remaining.count() <= 0)
        {
            Logger::error("Timeout waiting for database connection");
            throw std::runtime_error("Timeout waiting for database connection");
        }

        Logger::debug("Waiting for available connection...");
        condition_.wait_for(lock, remaining);

        if (is_shutdown_)
        {
            throw std::runtime_error("Connection pool is shut down");
        }
    }

    // Get connection from pool
    auto conn = available_connections_.front();
    available_connections_.pop();

    // Validate connection
    if (!isConnectionValid(conn))
    {
        Logger::warn("Invalid connection detected, creating new one");
        total_connections_--;
        lock.unlock();
        auto new_conn = createConnection();
        lock.lock();

        if (new_conn && new_conn->isConnected())
        {
            total_connections_++;
            return PooledConnection(new_conn, [this](auto c)
            {
                this->returnConnection(c);
            });
        }
        else
        {
            throw std::runtime_error("Failed to create replacement connection");
        }
    }

    Logger::debug("Connection acquired, available: {}, total: {}",
                 available_connections_.size(), total_connections_);

    return PooledConnection(conn, [this](auto c)
    {
        this->returnConnection(c);
    });
}

void DatabaseConnectionPool::returnConnection(std::shared_ptr<DatabaseConnection> conn)
{
    if (!conn)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    if (is_shutdown_)
    {
        total_connections_--;
        return;
    }

    // Validate connection before returning to pool
    if (isConnectionValid(conn))
    {
        available_connections_.push(conn);
        Logger::debug("Connection returned to pool, available: {}", available_connections_.size());
    }
    else
    {
        Logger::warn("Invalid connection not returned to pool");
        total_connections_--;
    }

    condition_.notify_one();
}

bool DatabaseConnectionPool::isConnectionValid(std::shared_ptr<DatabaseConnection> conn)
{
    if (!conn)
    {
        return false;
    }

    // Check if connection is still connected
    if (!conn->isConnected())
    {
        return false;
    }

    // Try a simple query to verify connection
    try
    {
        conn->executeQuery("SELECT 1");
        return true;
    }
    catch (...)
    {
        return false;
    }
}

void DatabaseConnectionPool::cleanupInvalidConnections()
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::queue<std::shared_ptr<DatabaseConnection>> valid_connections;
    size_t removed = 0;

    while (!available_connections_.empty())
    {
        auto conn = available_connections_.front();
        available_connections_.pop();

        if (isConnectionValid(conn))
        {
            valid_connections.push(conn);
        }
        else
        {
            total_connections_--;
            removed++;
        }
    }

    available_connections_ = std::move(valid_connections);

    if (removed > 0)
    {
        Logger::info("Cleaned up {} invalid connections", removed);
    }
}

size_t DatabaseConnectionPool::getAvailableConnections() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return available_connections_.size();
}

size_t DatabaseConnectionPool::getTotalConnections() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return total_connections_;
}

size_t DatabaseConnectionPool::getActiveConnections() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return total_connections_ - available_connections_.size();
}

void DatabaseConnectionPool::shutdown()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (is_shutdown_)
    {
        return;
    }

    Logger::info("Shutting down connection pool...");

    is_shutdown_ = true;

    // Clear all connections
    while (!available_connections_.empty())
    {
        available_connections_.pop();
    }

    total_connections_ = 0;

    condition_.notify_all();

    Logger::info("Connection pool shut down complete");
}
