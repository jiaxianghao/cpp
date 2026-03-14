#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H

#include <string>
#include <memory>
#include <chrono>
#include <optional>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <future>

// Simple Redis-like cache interface
class CacheManager
{
public:
    static CacheManager& getInstance();
    
    // Delete copy and move
    CacheManager(const CacheManager&) = delete;
    CacheManager& operator=(const CacheManager&) = delete;
    CacheManager(CacheManager&&) = delete;
    CacheManager& operator=(CacheManager&&) = delete;
    
    // Initialize cache with configuration
    bool initialize(const std::string& host = "localhost", 
                   int port = 6379,
                   int database = 0,
                   const std::string& password = "",
                   int connection_timeout = 5,
                   int socket_timeout = 5,
                   int pool_size = 10);
    
    // Basic cache operations
    bool set(const std::string& key, const std::string& value, 
             std::chrono::seconds ttl = std::chrono::seconds(3600));
    
    std::optional<std::string> get(const std::string& key);
    
    bool del(const std::string& key);
    
    bool exists(const std::string& key);
    
    // Batch operations
    bool mset(const std::unordered_map<std::string, std::string>& key_values,
              std::chrono::seconds ttl = std::chrono::seconds(3600));
    
    std::unordered_map<std::string, std::optional<std::string>> mget(
        const std::vector<std::string>& keys);
    
    // Increment/decrement operations
    long long incr(const std::string& key);
    long long decr(const std::string& key);
    long long incrBy(const std::string& key, long long increment);
    long long decrBy(const std::string& key, long long decrement);
    
    // Hash operations
    bool hset(const std::string& key, const std::string& field, const std::string& value);
    std::optional<std::string> hget(const std::string& key, const std::string& field);
    bool hdel(const std::string& key, const std::string& field);
    std::unordered_map<std::string, std::string> hgetall(const std::string& key);
    std::vector<std::string> hkeys(const std::string& key);
    std::vector<std::string> hvals(const std::string& key);
    
    // List operations
    long long lpush(const std::string& key, const std::string& value);
    long long rpush(const std::string& key, const std::string& value);
    std::optional<std::string> lpop(const std::string& key);
    std::optional<std::string> rpop(const std::string& key);
    std::vector<std::string> lrange(const std::string& key, long long start, long long stop);
    
    // Set operations
    bool sadd(const std::string& key, const std::string& member);
    bool srem(const std::string& key, const std::string& member);
    std::vector<std::string> smembers(const std::string& key);
    bool sismember(const std::string& key, const std::string& member);
    
    // Expiration operations
    bool expire(const std::string& key, std::chrono::seconds ttl);
    std::optional<long long> ttl(const std::string& key);
    bool persist(const std::string& key);
    
    // Statistics
    struct CacheStats {
        size_t total_keys = 0;
        size_t memory_usage = 0;
        size_t hit_count = 0;
        size_t miss_count = 0;
        double hit_rate = 0.0;
    };
    
    CacheStats getStats() const;
    void resetStats();
    
    // Cleanup expired keys
    void cleanupExpired();
    
    // Clear all cache
    void clear();
    
    // Shutdown cache
    void shutdown();
    
    // Check if cache is connected
    bool isConnected() const;
    
    // Get connection info
    std::string getConnectionInfo() const;
    
private:
    CacheManager() = default;
    ~CacheManager() = default;
    
    // Internal cache entry structure
    struct CacheEntry {
        std::string value;
        std::chrono::steady_clock::time_point expiry_time;
        bool has_expiry;
        
        CacheEntry(const std::string& val, std::chrono::seconds ttl)
            : value(val), has_expiry(ttl.count() > 0)
        {
            if (has_expiry) {
                expiry_time = std::chrono::steady_clock::now() + ttl;
            }
        }
        
        bool isExpired() const {
            if (!has_expiry) return false;
            return std::chrono::steady_clock::now() > expiry_time;
        }
    };
    
    mutable std::mutex mutex_;
    std::unordered_map<std::string, CacheEntry> cache_;
    
    // Statistics
    mutable size_t hit_count_ = 0;
    mutable size_t miss_count_ = 0;
    
    // Connection info (for Redis compatibility)
    std::string host_ = "localhost";
    int port_ = 6379;
    int database_ = 0;
    bool is_connected_ = false;
    
    // Hash maps for different data types
    std::unordered_map<std::string, std::unordered_map<std::string, CacheEntry>> hashes_;
    std::unordered_map<std::string, std::vector<std::string>> lists_;
    std::unordered_map<std::string, std::unordered_set<std::string>> sets_;
    
    // Internal helper methods
    bool isKeyExpired(const std::string& key) const;
    void removeExpiredKey(const std::string& key);
    std::string getCurrentTimestamp() const;
};

#endif // CACHE_MANAGER_H