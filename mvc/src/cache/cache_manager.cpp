#include "cache/cache_manager.h"
#include "utils/logger.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>

CacheManager& CacheManager::getInstance()
{
    static CacheManager instance;
    return instance;
}

bool CacheManager::initialize(const std::string& host, int port, int database,
                               const std::string& password, int connection_timeout,
                               int socket_timeout, int pool_size)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    host_ = host;
    port_ = port;
    database_ = database;
    is_connected_ = true;
    
    Logger::info("Cache manager initialized: {}:{}/database_{}", host, port, database);
    return true;
}

bool CacheManager::set(const std::string& key, const std::string& value, std::chrono::seconds ttl)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot set key: {}", key);
        return false;
    }
    
    try
    {
        cache_[key] = CacheEntry(value, ttl);
        Logger::debug("Cache set: {} = {} (TTL: {}s)", key, value, ttl.count());
        return true;
    }
    catch (const std::exception& e)
    {
        Logger::error("Failed to set cache key {}: {}", key, e.what());
        return false;
    }
}

std::optional<std::string> CacheManager::get(const std::string& key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot get key: {}", key);
        miss_count_++;
        return std::nullopt;
    }
    
    auto it = cache_.find(key);
    if (it == cache_.end())
    {
        Logger::debug("Cache miss for key: {}", key);
        miss_count_++;
        return std::nullopt;
    }
    
    // Check if entry is expired
    if (it->second.isExpired())
    {
        Logger::debug("Cache entry expired for key: {}", key);
        cache_.erase(it);
        miss_count_++;
        return std::nullopt;
    }
    
    Logger::debug("Cache hit for key: {}", key);
    hit_count_++;
    return it->second.value;
}

bool CacheManager::del(const std::string& key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot delete key: {}", key);
        return false;
    }
    
    size_t erased = cache_.erase(key);
    hashes_.erase(key);
    lists_.erase(key);
    sets_.erase(key);
    
    bool success = erased > 0;
    if (success)
    {
        Logger::debug("Cache key deleted: {}", key);
    }
    else
    {
        Logger::debug("Cache key not found for deletion: {}", key);
    }
    
    return success;
}

bool CacheManager::exists(const std::string& key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        return false;
    }
    
    auto it = cache_.find(key);
    if (it == cache_.end())
    {
        return false;
    }
    
    if (it->second.isExpired())
    {
        cache_.erase(it);
        return false;
    }
    
    return true;
}

bool CacheManager::mset(const std::unordered_map<std::string, std::string>& key_values,
                         std::chrono::seconds ttl)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot perform mset");
        return false;
    }
    
    try
    {
        for (const auto& [key, value] : key_values)
        {
            cache_[key] = CacheEntry(value, ttl);
        }
        
        Logger::debug("Cache mset completed for {} keys", key_values.size());
        return true;
    }
    catch (const std::exception& e)
    {
        Logger::error("Failed to perform mset: {}", e.what());
        return false;
    }
}

std::unordered_map<std::string, std::optional<std::string>> CacheManager::mget(
    const std::vector<std::string>& keys)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::unordered_map<std::string, std::optional<std::string>> results;
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot perform mget");
        for (const auto& key : keys)
        {
            results[key] = std::nullopt;
            miss_count_++;
        }
        return results;
    }
    
    for (const auto& key : keys)
    {
        auto it = cache_.find(key);
        if (it != cache_.end() && !it->second.isExpired())
        {
            results[key] = it->second.value;
            hit_count_++;
        }
        else
        {
            results[key] = std::nullopt;
            miss_count_++;
            
            // Remove expired entry
            if (it != cache_.end() && it->second.isExpired())
            {
                cache_.erase(it);
            }
        }
    }
    
    Logger::debug("Cache mget completed for {} keys", keys.size());
    return results;
}

long long CacheManager::incr(const std::string& key)
{
    return incrBy(key, 1);
}

long long CacheManager::decr(const std::string& key)
{
    return decrBy(key, 1);
}

long long CacheManager::incrBy(const std::string& key, long long increment)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot increment key: {}", key);
        return 0;
    }
    
    auto it = cache_.find(key);
    if (it == cache_.end() || it->second.isExpired())
    {
        // Create new counter
        cache_[key] = CacheEntry(std::to_string(increment), std::chrono::seconds(0));
        return increment;
    }
    
    try
    {
        long long current_value = std::stoll(it->second.value);
        current_value += increment;
        it->second.value = std::to_string(current_value);
        return current_value;
    }
    catch (const std::exception& e)
    {
        Logger::error("Failed to increment key {}: {}", key, e.what());
        return 0;
    }
}

long long CacheManager::decrBy(const std::string& key, long long decrement)
{
    return incrBy(key, -decrement);
}

bool CacheManager::hset(const std::string& key, const std::string& field, const std::string& value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot hset key: {}", key);
        return false;
    }
    
    hashes_[key][field] = CacheEntry(value, std::chrono::seconds(0));
    Logger::debug("Hash set: {}.{} = {}", key, field, value);
    return true;
}

std::optional<std::string> CacheManager::hget(const std::string& key, const std::string& field)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot hget key: {}", key);
        return std::nullopt;
    }
    
    auto hash_it = hashes_.find(key);
    if (hash_it == hashes_.end())
    {
        return std::nullopt;
    }
    
    auto field_it = hash_it->second.find(field);
    if (field_it == hash_it->second.end())
    {
        return std::nullopt;
    }
    
    if (field_it->second.isExpired())
    {
        hash_it->second.erase(field_it);
        if (hash_it->second.empty())
        {
            hashes_.erase(hash_it);
        }
        return std::nullopt;
    }
    
    return field_it->second.value;
}

bool CacheManager::hdel(const std::string& key, const std::string& field)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot hdel key: {}", key);
        return false;
    }
    
    auto hash_it = hashes_.find(key);
    if (hash_it == hashes_.end())
    {
        return false;
    }
    
    size_t erased = hash_it->second.erase(field);
    if (hash_it->second.empty())
    {
        hashes_.erase(hash_it);
    }
    
    return erased > 0;
}

std::unordered_map<std::string, std::string> CacheManager::hgetall(const std::string& key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::unordered_map<std::string, std::string> result;
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot hgetall key: {}", key);
        return result;
    }
    
    auto hash_it = hashes_.find(key);
    if (hash_it == hashes_.end())
    {
        return result;
    }
    
    // Remove expired fields and collect valid ones
    auto it = hash_it->second.begin();
    while (it != hash_it->second.end())
    {
        if (it->second.isExpired())
        {
            it = hash_it->second.erase(it);
        }
        else
        {
            result[it->first] = it->second.value;
            ++it;
        }
    }
    
    if (hash_it->second.empty())
    {
        hashes_.erase(hash_it);
    }
    
    return result;
}

std::vector<std::string> CacheManager::hkeys(const std::string& key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> result;
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot hkeys key: {}", key);
        return result;
    }
    
    auto hash_it = hashes_.find(key);
    if (hash_it == hashes_.end())
    {
        return result;
    }
    
    // Remove expired fields and collect keys
    auto it = hash_it->second.begin();
    while (it != hash_it->second.end())
    {
        if (it->second.isExpired())
        {
            it = hash_it->second.erase(it);
        }
        else
        {
            result.push_back(it->first);
            ++it;
        }
    }
    
    if (hash_it->second.empty())
    {
        hashes_.erase(hash_it);
    }
    
    return result;
}

std::vector<std::string> CacheManager::hvals(const std::string& key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> result;
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot hvals key: {}", key);
        return result;
    }
    
    auto hash_it = hashes_.find(key);
    if (hash_it == hashes_.end())
    {
        return result;
    }
    
    // Remove expired fields and collect values
    auto it = hash_it->second.begin();
    while (it != hash_it->second.end())
    {
        if (it->second.isExpired())
        {
            it = hash_it->second.erase(it);
        }
        else
        {
            result.push_back(it->second.value);
            ++it;
        }
    }
    
    if (hash_it->second.empty())
    {
        hashes_.erase(hash_it);
    }
    
    return result;
}

long long CacheManager::lpush(const std::string& key, const std::string& value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot lpush key: {}", key);
        return -1;
    }
    
    lists_[key].insert(lists_[key].begin(), value);
    Logger::debug("List left push: {} <- {}", key, value);
    return lists_[key].size();
}

long long CacheManager::rpush(const std::string& key, const std::string& value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot rpush key: {}", key);
        return -1;
    }
    
    lists_[key].push_back(value);
    Logger::debug("List right push: {} -> {}", key, value);
    return lists_[key].size();
}

std::optional<std::string> CacheManager::lpop(const std::string& key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot lpop key: {}", key);
        return std::nullopt;
    }
    
    auto list_it = lists_.find(key);
    if (list_it == lists_.end() || list_it->second.empty())
    {
        return std::nullopt;
    }
    
    std::string value = list_it->second.front();
    list_it->second.erase(list_it->second.begin());
    
    if (list_it->second.empty())
    {
        lists_.erase(list_it);
    }
    
    Logger::debug("List left pop: {} -> {}", key, value);
    return value;
}

std::optional<std::string> CacheManager::rpop(const std::string& key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot rpop key: {}", key);
        return std::nullopt;
    }
    
    auto list_it = lists_.find(key);
    if (list_it == lists_.end() || list_it->second.empty())
    {
        return std::nullopt;
    }
    
    std::string value = list_it->second.back();
    list_it->second.pop_back();
    
    if (list_it->second.empty())
    {
        lists_.erase(list_it);
    }
    
    Logger::debug("List right pop: {} <- {}", key, value);
    return value;
}

std::vector<std::string> CacheManager::lrange(const std::string& key, long long start, long long stop)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> result;
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot lrange key: {}", key);
        return result;
    }
    
    auto list_it = lists_.find(key);
    if (list_it == lists_.end())
    {
        return result;
    }
    
    const auto& list = list_it->second;
    long long list_size = static_cast<long long>(list.size());
    
    // Handle negative indices
    if (start < 0) start = list_size + start;
    if (stop < 0) stop = list_size + stop;
    
    // Clamp to valid range
    start = std::max(0LL, std::min(start, list_size));
    stop = std::max(0LL, std::min(stop + 1, list_size));
    
    if (start >= stop) return result;
    
    for (long long i = start; i < stop; ++i)
    {
        result.push_back(list[i]);
    }
    
    Logger::debug("List range: {}[{}:{}] returned {} items", key, start, stop - 1, result.size());
    return result;
}

bool CacheManager::sadd(const std::string& key, const std::string& member)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot sadd key: {}", key);
        return false;
    }
    
    auto [it, inserted] = sets_[key].insert(member);
    Logger::debug("Set add: {} <- {} (new: {})", key, member, inserted);
    return inserted;
}

bool CacheManager::srem(const std::string& key, const std::string& member)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot srem key: {}", key);
        return false;
    }
    
    auto set_it = sets_.find(key);
    if (set_it == sets_.end())
    {
        return false;
    }
    
    size_t erased = set_it->second.erase(member);
    if (set_it->second.empty())
    {
        sets_.erase(set_it);
    }
    
    Logger::debug("Set remove: {} - {} (removed: {})", key, member, erased > 0);
    return erased > 0;
}

std::vector<std::string> CacheManager::smembers(const std::string& key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> result;
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot smembers key: {}", key);
        return result;
    }
    
    auto set_it = sets_.find(key);
    if (set_it != sets_.end())
    {
        result.assign(set_it->second.begin(), set_it->second.end());
    }
    
    Logger::debug("Set members: {} has {} members", key, result.size());
    return result;
}

bool CacheManager::sismember(const std::string& key, const std::string& member)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot sismember key: {}", key);
        return false;
    }
    
    auto set_it = sets_.find(key);
    if (set_it == sets_.end())
    {
        return false;
    }
    
    bool is_member = set_it->second.count(member) > 0;
    Logger::debug("Set is member: {} contains {}: {}", key, member, is_member);
    return is_member;
}

bool CacheManager::expire(const std::string& key, std::chrono::seconds ttl)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot expire key: {}", key);
        return false;
    }
    
    auto it = cache_.find(key);
    if (it == cache_.end())
    {
        return false;
    }
    
    it->second.expiry_time = std::chrono::steady_clock::now() + ttl;
    it->second.has_expiry = true;
    
    Logger::debug("Key expiration set: {} (TTL: {}s)", key, ttl.count());
    return true;
}

std::optional<long long> CacheManager::ttl(const std::string& key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot get TTL for key: {}", key);
        return std::nullopt;
    }
    
    auto it = cache_.find(key);
    if (it == cache_.end())
    {
        return std::nullopt;
    }
    
    if (!it->second.has_expiry)
    {
        return -1LL; // Key exists but has no expiration
    }
    
    auto now = std::chrono::steady_clock::now();
    if (now >= it->second.expiry_time)
    {
        return -2LL; // Key has expired
    }
    
    auto remaining = std::chrono::duration_cast<std::chrono::seconds>(
        it->second.expiry_time - now);
    
    return remaining.count();
}

bool CacheManager::persist(const std::string& key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_connected_)
    {
        Logger::warn("Cache not connected, cannot persist key: {}", key);
        return false;
    }
    
    auto it = cache_.find(key);
    if (it == cache_.end())
    {
        return false;
    }
    
    it->second.has_expiry = false;
    Logger::debug("Key persistence set: {}", key);
    return true;
}

CacheManager::CacheStats CacheManager::getStats() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    CacheStats stats;
    stats.total_keys = cache_.size() + hashes_.size() + lists_.size() + sets_.size();
    
    // Calculate memory usage (rough estimate)
    for (const auto& [key, entry] : cache_)
    {
        stats.memory_usage += key.size() + entry.value.size() + sizeof(CacheEntry);
    }
    
    stats.hit_count = hit_count_;
    stats.miss_count = miss_count_;
    
    size_t total_requests = hit_count_ + miss_count_;
    if (total_requests > 0)
    {
        stats.hit_rate = static_cast<double>(hit_count_) / total_requests * 100.0;
    }
    
    return stats;
}

void CacheManager::resetStats()
{
    std::lock_guard<std::mutex> lock(mutex_);
    hit_count_ = 0;
    miss_count_ = 0;
    Logger::info("Cache statistics reset");
}

void CacheManager::cleanupExpired()
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t expired_count = 0;
    
    // Clean up expired regular keys
    auto it = cache_.begin();
    while (it != cache_.end())
    {
        if (it->second.isExpired())
        {
            it = cache_.erase(it);
            expired_count++;
        }
        else
        {
            ++it;
        }
    }
    
    // Clean up expired hash fields
    for (auto& [hash_key, hash_map] : hashes_)
    {
        auto field_it = hash_map.begin();
        while (field_it != hash_map.end())
        {
            if (field_it->second.isExpired())
            {
                field_it = hash_map.erase(field_it);
            }
            else
            {
                ++field_it;
            }
        }
    }
    
    // Remove empty hashes
    auto hash_it = hashes_.begin();
    while (hash_it != hashes_.end())
    {
        if (hash_it->second.empty())
        {
            hash_it = hashes_.erase(hash_it);
        }
        else
        {
            ++hash_it;
        }
    }
    
    if (expired_count > 0)
    {
        Logger::info("Cleaned up {} expired cache entries", expired_count);
    }
}

void CacheManager::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    cache_.clear();
    hashes_.clear();
    lists_.clear();
    sets_.clear();
    
    Logger::info("Cache cleared completely");
}

void CacheManager::shutdown()
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    is_connected_ = false;
    clear();
    
    Logger::info("Cache manager shut down");
}

bool CacheManager::isConnected() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return is_connected_;
}

std::string CacheManager::getConnectionInfo() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ostringstream info;
    info << "Cache: " << host_ << ":" << port_ << "/database_" << database_;
    info << " (connected: " << (is_connected_ ? "yes" : "no") << ")";
    return info.str();
}

bool CacheManager::isKeyExpired(const std::string& key) const
{
    auto it = cache_.find(key);
    if (it != cache_.end() && it->second.isExpired())
    {
        return true;
    }
    return false;
}

void CacheManager::removeExpiredKey(const std::string& key)
{
    auto it = cache_.find(key);
    if (it != cache_.end() && it->second.isExpired())
    {
        cache_.erase(it);
    }
}

std::string CacheManager::getCurrentTimestamp() const
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}