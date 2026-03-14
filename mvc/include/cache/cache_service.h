#ifndef CACHE_SERVICE_H
#define CACHE_SERVICE_H

#include "cache/cache_manager.h"
#include <string>
#include <chrono>
#include <functional>
#include <optional>

// Service class for managing cache operations with business logic
class CacheService
{
public:
    static CacheService& getInstance();
    
    // Delete copy and move
    CacheService(const CacheService&) = delete;
    CacheService& operator=(const CacheService&) = delete;
    CacheService(CacheService&&) = delete;
    CacheService& operator=(CacheService&&) = delete;
    
    // Initialize cache service with configuration
    bool initialize(const std::string& config_file = "");
    
    // Cache-aside pattern: Try cache first, if miss then load from source
    template<typename T>
    std::optional<T> getWithFallback(const std::string& key, 
                                      std::function<std::optional<T>()> loader,
                                      std::function<std::string(const T&)> serializer,
                                      std::function<std::optional<T>(const std::string&)> deserializer,
                                      std::chrono::seconds ttl = std::chrono::seconds(3600))
    {
        // Try to get from cache
        auto cached_value = cache_manager_->get(key);
        if (cached_value.has_value())
        {
            auto result = deserializer(cached_value.value());
            if (result.has_value())
            {
                return result;
            }
        }
        
        // Cache miss, load from source
        auto loaded_value = loader();
        if (loaded_value.has_value())
        {
            // Serialize and cache the value
            std::string serialized = serializer(loaded_value.value());
            cache_manager_->set(key, serialized, ttl);
            return loaded_value;
        }
        
        return std::nullopt;
    }
    
    // Simple string operations
    bool set(const std::string& key, const std::string& value, 
             std::chrono::seconds ttl = std::chrono::seconds(3600));
    
    std::optional<std::string> get(const std::string& key);
    
    bool del(const std::string& key);
    
    bool exists(const std::string& key);
    
    // User-specific cache operations
    bool cacheUser(const std::string& user_id, const std::string& user_data,
                    std::chrono::seconds ttl = std::chrono::seconds(1800)); // 30 minutes
    
    std::optional<std::string> getCachedUser(const std::string& user_id);
    
    bool invalidateUser(const std::string& user_id);
    
    // Product-specific cache operations
    bool cacheProduct(const std::string& product_id, const std::string& product_data,
                      std::chrono::seconds ttl = std::chrono::seconds(3600)); // 1 hour
    
    std::optional<std::string> getCachedProduct(const std::string& product_id);
    
    bool invalidateProduct(const std::string& product_id);
    
    // Order-specific cache operations
    bool cacheOrder(const std::string& order_id, const std::string& order_data,
                    std::chrono::seconds ttl = std::chrono::seconds(900)); // 15 minutes
    
    std::optional<std::string> getCachedOrder(const std::string& order_id);
    
    bool invalidateOrder(const std::string& order_id);
    
    // List operations for collections
    bool cacheUserList(const std::string& list_key, const std::vector<std::string>& user_ids,
                       std::chrono::seconds ttl = std::chrono::seconds(1800));
    
    std::optional<std::vector<std::string>> getCachedUserList(const std::string& list_key);
    
    bool invalidateUserList(const std::string& list_key);
    
    // Search result caching
    bool cacheSearchResults(const std::string& search_key, const std::string& results,
                            std::chrono::seconds ttl = std::chrono::seconds(300)); // 5 minutes
    
    std::optional<std::string> getCachedSearchResults(const std::string& search_key);
    
    bool invalidateSearchResults(const std::string& search_key);
    
    // Statistics and monitoring
    struct CacheStats {
        size_t total_keys = 0;
        size_t memory_usage = 0;
        size_t hit_count = 0;
        size_t miss_count = 0;
        double hit_rate = 0.0;
        size_t user_cache_entries = 0;
        size_t product_cache_entries = 0;
        size_t order_cache_entries = 0;
    };
    
    CacheStats getStats() const;
    
    // Cleanup operations
    void cleanupExpired();
    void clearAll();
    void clearUserCache();
    void clearProductCache();
    void clearOrderCache();
    
    // Cache warming
    void warmCache(const std::vector<std::string>& user_ids,
                   const std::vector<std::string>& product_ids = {});
    
    // Health check
    bool isHealthy() const;
    
    std::string getStatus() const;
    
private:
    CacheManager* cache_manager_ = nullptr;
    bool initialized_ = false;
    
    // Key generators for different object types
    std::string generateUserKey(const std::string& user_id) const;
    std::string generateProductKey(const std::string& product_id) const;
    std::string generateOrderKey(const std::string& order_id) const;
    std::string generateSearchKey(const std::string& search_query) const;
    std::string generateUserListKey(const std::string& list_type) const;
    
    // Utility methods
    bool checkInitialization();
    void logCacheOperation(const std::string& operation, const std::string& key, bool success) const;
};

#endif // CACHE_SERVICE_H