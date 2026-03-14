#include <gtest/gtest.h>
#include "cache/cache_manager.h"
#include "cache/cache_service.h"
#include <chrono>
#include <thread>

class CacheManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto& cache_manager = CacheManager::getInstance();
        cache_manager.initialize("localhost", 6379, 0, "", 5, 5, 10);
    }

    void TearDown() override {
        auto& cache_manager = CacheManager::getInstance();
        cache_manager.shutdown();
    }
};

TEST_F(CacheManagerTest, BasicSetAndGet) {
    auto& cache = CacheManager::getInstance();
    
    // Test basic set and get
    EXPECT_TRUE(cache.set("test_key", "test_value"));
    
    auto result = cache.get("test_key");
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "test_value");
}

TEST_F(CacheManagerTest, NonExistentKey) {
    auto& cache = CacheManager::getInstance();
    
    auto result = cache.get("non_existent_key");
    EXPECT_FALSE(result.has_value());
}

TEST_F(CacheManagerTest, DeleteOperation) {
    auto& cache = CacheManager::getInstance();
    
    // Set a key
    EXPECT_TRUE(cache.set("delete_key", "delete_value"));
    
    // Verify it exists
    EXPECT_TRUE(cache.exists("delete_key"));
    
    // Delete it
    EXPECT_TRUE(cache.del("delete_key"));
    
    // Verify it's gone
    EXPECT_FALSE(cache.exists("delete_key"));
    EXPECT_FALSE(cache.get("delete_key").has_value());
}

TEST_F(CacheManagerTest, ExpirationTest) {
    auto& cache = CacheManager::getInstance();
    
    // Set key with 1 second TTL
    EXPECT_TRUE(cache.set("expire_key", "expire_value", std::chrono::seconds(1)));
    
    // Verify it exists immediately
    EXPECT_TRUE(cache.exists("expire_key"));
    
    // Wait for expiration
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Verify it's expired
    EXPECT_FALSE(cache.exists("expire_key"));
    EXPECT_FALSE(cache.get("expire_key").has_value());
}

TEST_F(CacheManagerTest, BatchOperations) {
    auto& cache = CacheManager::getInstance();
    
    // Test mset
    std::unordered_map<std::string, std::string> key_values = {
        {"key1", "value1"},
        {"key2", "value2"},
        {"key3", "value3"}
    };
    
    EXPECT_TRUE(cache.mset(key_values));
    
    // Test mget
    std::vector<std::string> keys = {"key1", "key2", "key3", "non_existent"};
    auto results = cache.mget(keys);
    
    EXPECT_EQ(results.size(), 4);
    EXPECT_TRUE(results["key1"].has_value());
    EXPECT_EQ(results["key1"].value(), "value1");
    EXPECT_TRUE(results["key2"].has_value());
    EXPECT_EQ(results["key2"].value(), "value2");
    EXPECT_TRUE(results["key3"].has_value());
    EXPECT_EQ(results["key3"].value(), "value3");
    EXPECT_FALSE(results["non_existent"].has_value());
}

TEST_F(CacheManagerTest, IncrementDecrement) {
    auto& cache = CacheManager::getInstance();
    
    // Test increment
    EXPECT_EQ(cache.incr("counter"), 1);
    EXPECT_EQ(cache.incr("counter"), 2);
    EXPECT_EQ(cache.incrBy("counter", 5), 7);
    
    // Test decrement
    EXPECT_EQ(cache.decr("counter"), 6);
    EXPECT_EQ(cache.decrBy("counter", 3), 3);
}

TEST_F(CacheManagerTest, HashOperations) {
    auto& cache = CacheManager::getInstance();
    
    // Test hset and hget
    EXPECT_TRUE(cache.hset("user:1", "name", "John"));
    EXPECT_TRUE(cache.hset("user:1", "email", "john@example.com"));
    
    auto name = cache.hget("user:1", "name");
    EXPECT_TRUE(name.has_value());
    EXPECT_EQ(name.value(), "John");
    
    auto email = cache.hget("user:1", "email");
    EXPECT_TRUE(email.has_value());
    EXPECT_EQ(email.value(), "john@example.com");
    
    // Test hgetall
    auto all_fields = cache.hgetall("user:1");
    EXPECT_EQ(all_fields.size(), 2);
    EXPECT_EQ(all_fields["name"], "John");
    EXPECT_EQ(all_fields["email"], "john@example.com");
    
    // Test hdel
    EXPECT_TRUE(cache.hdel("user:1", "email"));
    EXPECT_FALSE(cache.hget("user:1", "email").has_value());
}

TEST_F(CacheManagerTest, ListOperations) {
    auto& cache = CacheManager::getInstance();
    
    // Test lpush and rpush
    EXPECT_EQ(cache.lpush("mylist", "first"), 1);
    EXPECT_EQ(cache.rpush("mylist", "second"), 2);
    EXPECT_EQ(cache.rpush("mylist", "third"), 3);
    
    // Test lrange
    auto items = cache.lrange("mylist", 0, -1);
    EXPECT_EQ(items.size(), 3);
    EXPECT_EQ(items[0], "first");
    EXPECT_EQ(items[1], "second");
    EXPECT_EQ(items[2], "third");
    
    // Test lpop and rpop
    auto first = cache.lpop("mylist");
    EXPECT_TRUE(first.has_value());
    EXPECT_EQ(first.value(), "first");
    
    auto last = cache.rpop("mylist");
    EXPECT_TRUE(last.has_value());
    EXPECT_EQ(last.value(), "third");
}

TEST_F(CacheManagerTest, SetOperations) {
    auto& cache = CacheManager::getInstance();
    
    // Test sadd
    EXPECT_TRUE(cache.sadd("myset", "member1"));
    EXPECT_TRUE(cache.sadd("myset", "member2"));
    EXPECT_TRUE(cache.sadd("myset", "member3"));
    EXPECT_FALSE(cache.sadd("myset", "member1")); // Duplicate
    
    // Test smembers
    auto members = cache.smembers("myset");
    EXPECT_EQ(members.size(), 3);
    
    // Test sismember
    EXPECT_TRUE(cache.sismember("myset", "member1"));
    EXPECT_FALSE(cache.sismember("myset", "member4"));
    
    // Test srem
    EXPECT_TRUE(cache.srem("myset", "member2"));
    EXPECT_FALSE(cache.sismember("myset", "member2"));
}

TEST_F(CacheManagerTest, Statistics) {
    auto& cache = CacheManager::getInstance();
    
    // Perform some operations
    cache.set("key1", "value1");
    cache.get("key1"); // hit
    cache.get("non_existent"); // miss
    
    auto stats = cache.getStats();
    EXPECT_GT(stats.total_keys, 0);
    EXPECT_GT(stats.hit_count, 0);
    EXPECT_GT(stats.miss_count, 0);
    EXPECT_GT(stats.hit_rate, 0.0);
}

TEST_F(CacheManagerTest, CleanupExpired) {
    auto& cache = CacheManager::getInstance();
    
    // Add some expired keys
    cache.set("expire1", "value1", std::chrono::seconds(1));
    cache.set("expire2", "value2", std::chrono::seconds(1));
    cache.set("keep1", "value3"); // No expiration
    
    // Wait for expiration
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Add another key to ensure cleanup processes mixed expired/unexpired keys
    cache.set("keep2", "value4");
    
    // Cleanup expired keys
    cache.cleanupExpired();
    
    // Verify expired keys are gone
    EXPECT_FALSE(cache.exists("expire1"));
    EXPECT_FALSE(cache.exists("expire2"));
    EXPECT_TRUE(cache.exists("keep1"));
    EXPECT_TRUE(cache.exists("keep2"));
}

TEST_F(CacheManagerTest, ConnectionStatus) {
    auto& cache = CacheManager::getInstance();
    
    EXPECT_TRUE(cache.isConnected());
    
    std::string connection_info = cache.getConnectionInfo();
    EXPECT_FALSE(connection_info.empty());
    EXPECT_NE(connection_info.find("localhost"), std::string::npos);
}

// CacheService tests
class CacheServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto& cache_service = CacheService::getInstance();
        cache_service.initialize();
    }

    void TearDown() override {
        auto& cache_service = CacheService::getInstance();
        // No explicit shutdown needed for singleton
    }
};

TEST_F(CacheServiceTest, UserCacheOperations) {
    auto& cache_service = CacheService::getInstance();
    
    // Test user caching
    std::string user_data = "{\"id\":1,\"username\":\"testuser\",\"email\":\"test@example.com\"}";
    
    EXPECT_TRUE(cache_service.cacheUser("1", user_data));
    
    auto cached_data = cache_service.getCachedUser("1");
    EXPECT_TRUE(cached_data.has_value());
    EXPECT_EQ(cached_data.value(), user_data);
    
    // Test invalidation
    EXPECT_TRUE(cache_service.invalidateUser("1"));
    EXPECT_FALSE(cache_service.getCachedUser("1").has_value());
}

TEST_F(CacheServiceTest, ProductCacheOperations) {
    auto& cache_service = CacheService::getInstance();
    
    // Test product caching
    std::string product_data = "{\"id\":1,\"name\":\"Test Product\",\"price\":99.99}";
    
    EXPECT_TRUE(cache_service.cacheProduct("1", product_data));
    
    auto cached_data = cache_service.getCachedProduct("1");
    EXPECT_TRUE(cached_data.has_value());
    EXPECT_EQ(cached_data.value(), product_data);
    
    // Test invalidation
    EXPECT_TRUE(cache_service.invalidateProduct("1"));
    EXPECT_FALSE(cache_service.getCachedProduct("1").has_value());
}

TEST_F(CacheServiceTest, OrderCacheOperations) {
    auto& cache_service = CacheService::getInstance();
    
    // Test order caching
    std::string order_data = "{\"id\":1,\"user_id\":1,\"total\":199.99,\"status\":\"pending\"}";
    
    EXPECT_TRUE(cache_service.cacheOrder("1", order_data));
    
    auto cached_data = cache_service.getCachedOrder("1");
    EXPECT_TRUE(cached_data.has_value());
    EXPECT_EQ(cached_data.value(), order_data);
    
    // Test invalidation
    EXPECT_TRUE(cache_service.invalidateOrder("1"));
    EXPECT_FALSE(cache_service.getCachedOrder("1").has_value());
}

TEST_F(CacheServiceTest, UserListCacheOperations) {
    auto& cache_service = CacheService::getInstance();
    
    // Test user list caching
    std::vector<std::string> user_ids = {"1", "2", "3", "4", "5"};
    
    EXPECT_TRUE(cache_service.cacheUserList("active_users", user_ids));
    
    auto cached_list = cache_service.getCachedUserList("active_users");
    EXPECT_TRUE(cached_list.has_value());
    EXPECT_EQ(cached_list.value().size(), user_ids.size());
    
    // Test invalidation
    EXPECT_TRUE(cache_service.invalidateUserList("active_users"));
    EXPECT_FALSE(cache_service.getCachedUserList("active_users").has_value());
}

TEST_F(CacheServiceTest, SearchResultCacheOperations) {
    auto& cache_service = CacheService::getInstance();
    
    // Test search result caching
    std::string search_results = "[{\"id\":1,\"username\":\"user1\"},{\"id\":2,\"username\":\"user2\"}]";
    
    EXPECT_TRUE(cache_service.cacheSearchResults("search:john", search_results));
    
    auto cached_results = cache_service.getCachedSearchResults("search:john");
    EXPECT_TRUE(cached_results.has_value());
    EXPECT_EQ(cached_results.value(), search_results);
    
    // Test invalidation
    EXPECT_TRUE(cache_service.invalidateSearchResults("search:john"));
    EXPECT_FALSE(cache_service.getCachedSearchResults("search:john").has_value());
}

TEST_F(CacheServiceTest, CacheStats) {
    auto& cache_service = CacheService::getInstance();
    
    // Add some cache entries
    cache_service.cacheUser("1", "{\"id\":1}");
    cache_service.cacheProduct("1", "{\"id\":1}");
    
    auto stats = cache_service.getStats();
    EXPECT_GT(stats.total_keys, 0);
    EXPECT_GT(stats.hit_count, 0);
    EXPECT_GT(stats.miss_count, 0);
}

TEST_F(CacheServiceTest, HealthCheck) {
    auto& cache_service = CacheService::getInstance();
    
    EXPECT_TRUE(cache_service.isHealthy());
    
    std::string status = cache_service.getStatus();
    EXPECT_FALSE(status.empty());
}