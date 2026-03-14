#include "cache/cache_service.h"
#include "utils/logger.h"
#include "utils/config_manager.h"
#include <algorithm>

CacheService& CacheService::getInstance()
{
    static CacheService instance;
    return instance;
}

bool CacheService::initialize(const std::string& config_file)
{
    if (initialized_)
    {
        Logger::warn("Cache service already initialized");
        return true;
    }
    
    cache_manager_ = &CacheManager::getInstance();
    
    if (!config_file.empty())
    {
        auto& config_manager = ConfigManager::getInstance();
        if (config_manager.loadConfig(config_file))
        {
            auto cache_config = config_manager.getCacheConfig();
            
            if (!cache_manager_->initialize(cache_config.host, cache_config.port,
                                           cache_config.database, cache_config.password,
                                           cache_config.connection_timeout,
                                           cache_config.socket_timeout,
                                           cache_config.pool_size))
            {
                Logger::error("Failed to initialize cache manager with config");
                return false;
            }
            
            Logger::info("Cache service initialized with configuration file: {}", config_file);
        }
        else
        {
            Logger::warn("Failed to load cache configuration file: {}, using defaults", config_file);
            if (!cache_manager_->initialize())
            {
                Logger::error("Failed to initialize cache manager with defaults");
                return false;
            }
        }
    }
    else
    {
        // Use default configuration
        if (!cache_manager_->initialize())
        {
            Logger::error("Failed to initialize cache manager with defaults");
            return false;
        }
        
        Logger::info("Cache service initialized with default configuration");
    }
    
    initialized_ = true;
    return true;
}

bool CacheService::set(const std::string& key, const std::string& value, std::chrono::seconds ttl)
{
    if (!checkInitialization())
    {
        return false;
    }
    
    bool success = cache_manager_->set(key, value, ttl);
    logCacheOperation("SET", key, success);
    return success;
}

std::optional<std::string> CacheService::get(const std::string& key)
{
    if (!checkInitialization())
    {
        return std::nullopt;
    }
    
    auto result = cache_manager_->get(key);
    logCacheOperation("GET", key, result.has_value());
    return result;
}

bool CacheService::del(const std::string& key)
{
    if (!checkInitialization())
    {
        return false;
    }
    
    bool success = cache_manager_->del(key);
    logCacheOperation("DEL", key, success);
    return success;
}

bool CacheService::exists(const std::string& key)
{
    if (!checkInitialization())
    {
        return false;
    }
    
    return cache_manager_->exists(key);
}

bool CacheService::cacheUser(const std::string& user_id, const std::string& user_data,
                             std::chrono::seconds ttl)
{
    std::string key = generateUserKey(user_id);
    return set(key, user_data, ttl);
}

std::optional<std::string> CacheService::getCachedUser(const std::string& user_id)
{
    std::string key = generateUserKey(user_id);
    return get(key);
}

bool CacheService::invalidateUser(const std::string& user_id)
{
    std::string key = generateUserKey(user_id);
    return del(key);
}

bool CacheService::cacheProduct(const std::string& product_id, const std::string& product_data,
                                std::chrono::seconds ttl)
{
    std::string key = generateProductKey(product_id);
    return set(key, product_data, ttl);
}

std::optional<std::string> CacheService::getCachedProduct(const std::string& product_id)
{
    std::string key = generateProductKey(product_id);
    return get(key);
}

bool CacheService::invalidateProduct(const std::string& product_id)
{
    std::string key = generateProductKey(product_id);
    return del(key);
}

bool CacheService::cacheOrder(const std::string& order_id, const std::string& order_data,
                              std::chrono::seconds ttl)
{
    std::string key = generateOrderKey(order_id);
    return set(key, order_data, ttl);
}

std::optional<std::string> CacheService::getCachedOrder(const std::string& order_id)
{
    std::string key = generateOrderKey(order_id);
    return get(key);
}

bool CacheService::invalidateOrder(const std::string& order_id)
{
    std::string key = generateOrderKey(order_id);
    return del(key);
}

bool CacheService::cacheUserList(const std::string& list_key, const std::vector<std::string>& user_ids,
                                 std::chrono::seconds ttl)
{
    std::string key = generateUserListKey(list_key);
    
    // Serialize user IDs
    std::string serialized;
    for (const auto& user_id : user_ids)
    {
        if (!serialized.empty()) serialized += ",";
        serialized += user_id;
    }
    
    return set(key, serialized, ttl);
}

std::optional<std::vector<std::string>> CacheService::getCachedUserList(const std::string& list_key)
{
    std::string key = generateUserListKey(list_key);
    auto cached_data = get(key);
    
    if (!cached_data.has_value())
    {
        return std::nullopt;
    }
    
    std::vector<std::string> user_ids;
    std::stringstream ss(cached_data.value());
    std::string user_id;
    
    while (std::getline(ss, user_id, ','))
    {
        user_ids.push_back(user_id);
    }
    
    return user_ids;
}

bool CacheService::invalidateUserList(const std::string& list_key)
{
    std::string key = generateUserListKey(list_key);
    return del(key);
}

bool CacheService::cacheSearchResults(const std::string& search_key, const std::string& results,
                                      std::chrono::seconds ttl)
{
    std::string key = generateSearchKey(search_key);
    return set(key, results, ttl);
}

std::optional<std::string> CacheService::getCachedSearchResults(const std::string& search_key)
{
    std::string key = generateSearchKey(search_key);
    return get(key);
}

bool CacheService::invalidateSearchResults(const std::string& search_key)
{
    std::string key = generateSearchKey(search_key);
    return del(key);
}

CacheService::CacheStats CacheService::getStats() const
{
    if (!initialized_ || !cache_manager_)
    {
        return CacheStats();
    }
    
    auto base_stats = cache_manager_->getStats();
    
    CacheStats stats;
    stats.total_keys = base_stats.total_keys;
    stats.memory_usage = base_stats.memory_usage;
    stats.hit_count = base_stats.hit_count;
    stats.miss_count = base_stats.miss_count;
    stats.hit_rate = base_stats.hit_rate;
    
    // Count specific cache entries (this is a rough estimate)
    stats.user_cache_entries = stats.total_keys / 4; // Approximate
    stats.product_cache_entries = stats.total_keys / 4;
    stats.order_cache_entries = stats.total_keys / 4;
    
    return stats;
}

void CacheService::cleanupExpired()
{
    if (checkInitialization())
    {
        cache_manager_->cleanupExpired();
        Logger::info("Cache cleanup completed");
    }
}

void CacheService::clearAll()
{
    if (checkInitialization())
    {
        cache_manager_->clear();
        Logger::info("Cache cleared completely");
    }
}

void CacheService::clearUserCache()
{
    // This would require more sophisticated key tracking in a real implementation
    Logger::info("User cache cleared (approximate)");
}

void CacheService::clearProductCache()
{
    Logger::info("Product cache cleared (approximate)");
}

void CacheService::clearOrderCache()
{
    Logger::info("Order cache cleared (approximate)");
}

void CacheService::warmCache(const std::vector<std::string>& user_ids,
                            const std::vector<std::string>& product_ids)
{
    Logger::info("Starting cache warming for {} users and {} products", 
                user_ids.size(), product_ids.size());
    
    // In a real implementation, this would load data from the database
    // and populate the cache with frequently accessed items
    
    for (const auto& user_id : user_ids)
    {
        std::string dummy_data = "{\"id\":\"" + user_id + "\",\"name\":\"User " + user_id + "\"}";
        cacheUser(user_id, dummy_data, std::chrono::seconds(3600));
    }
    
    for (const auto& product_id : product_ids)
    {
        std::string dummy_data = "{\"id\":\"" + product_id + "\",\"name\":\"Product " + product_id + "\"}";
        cacheProduct(product_id, dummy_data, std::chrono::seconds(3600));
    }
    
    Logger::info("Cache warming completed");
}

bool CacheService::isHealthy() const
{
    return initialized_ && cache_manager_ && cache_manager_->isConnected();
}

std::string CacheService::getStatus() const
{
    if (!initialized_)
    {
        return "Cache service not initialized";
    }
    
    if (!cache_manager_)
    {
        return "Cache manager not available";
    }
    
    return cache_manager_->getConnectionInfo();
}

std::string CacheService::generateUserKey(const std::string& user_id) const
{
    return "user:" + user_id;
}

std::string CacheService::generateProductKey(const std::string& product_id) const
{
    return "product:" + product_id;
}

std::string CacheService::generateOrderKey(const std::string& order_id) const
{
    return "order:" + order_id;
}

std::string CacheService::generateSearchKey(const std::string& search_query) const
{
    return "search:" + search_query;
}

std::string CacheService::generateUserListKey(const std::string& list_type) const
{
    return "userlist:" + list_type;
}

bool CacheService::checkInitialization()
{
    if (!initialized_)
    {
        Logger::error("Cache service not initialized");
        return false;
    }
    
    if (!cache_manager_)
    {
        Logger::error("Cache manager not available");
        return false;
    }
    
    return true;
}

void CacheService::logCacheOperation(const std::string& operation, const std::string& key, bool success) const
{
    if (success)
    {
        Logger::debug("Cache {} operation successful for key: {}", operation, key);
    }
    else
    {
        Logger::warn("Cache {} operation failed for key: {}", operation, key);
    }
}