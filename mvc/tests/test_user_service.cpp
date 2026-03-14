#include <gtest/gtest.h>
#include "services/user_service.h"
#include "repositories/user_repository.h"
#include "cache/cache_service.h"
#include "database/database_connection.h"
#include <memory>

class MockUserRepository : public UserRepository {
public:
    MockUserRepository() : UserRepository(nullptr) {}
    
    int create(const User& user) override {
        last_created_user_ = user;
        return next_user_id_++;
    }
    
    User getById(int id) override {
        if (id == 1) {
            User user("testuser", "test@example.com");
            user.setId(1);
            return user;
        }
        return User();
    }
    
    User getByUsername(const std::string& username) override {
        if (username == "testuser") {
            User user("testuser", "test@example.com");
            user.setId(1);
            return user;
        }
        return User();
    }
    
    User getByEmail(const std::string& email) override {
        if (email == "test@example.com") {
            User user("testuser", "test@example.com");
            user.setId(1);
            return user;
        }
        return User();
    }
    
    std::vector<User> getAll() override {
        std::vector<User> users;
        User user1("user1", "user1@example.com");
        user1.setId(1);
        User user2("user2", "user2@example.com");
        user2.setId(2);
        users.push_back(user1);
        users.push_back(user2);
        return users;
    }
    
    bool update(const User& user) override {
        last_updated_user_ = user;
        return true;
    }
    
    bool deleteById(int id) override {
        return id == 1;
    }
    
    bool exists(int id) override {
        return id == 1;
    }
    
    bool existsByUsername(const std::string& username) override {
        return username == "existinguser";
    }
    
    bool existsByEmail(const std::string& email) override {
        return email == "existing@example.com";
    }
    
    std::vector<User> searchByUsername(const std::string& pattern) override {
        std::vector<User> users;
        if (pattern == "test") {
            User user("testuser", "test@example.com");
            user.setId(1);
            users.push_back(user);
        }
        return users;
    }
    
    std::vector<User> searchByEmail(const std::string& pattern) override {
        std::vector<User> users;
        if (pattern == "test") {
            User user("testuser", "test@example.com");
            user.setId(1);
            users.push_back(user);
        }
        return users;
    }
    
    int getCount() override {
        return 2;
    }
    
    // Helper methods for test verification
    User getLastCreatedUser() const { return last_created_user_; }
    User getLastUpdatedUser() const { return last_updated_user_; }
    
private:
    User last_created_user_;
    User last_updated_user_;
    int next_user_id_ = 1;
};

class UserServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_repo_ = std::make_shared<MockUserRepository>();
        cache_service_ = std::make_shared<CacheService>();
        cache_service_->initialize(); // Initialize with default config
        user_service_ = std::make_unique<UserService>(mock_repo_, cache_service_);
    }

    void TearDown() override {
        user_service_.reset();
        cache_service_.reset();
        mock_repo_.reset();
    }

    std::shared_ptr<MockUserRepository> mock_repo_;
    std::shared_ptr<CacheService> cache_service_;
    std::unique_ptr<UserService> user_service_;
};

TEST_F(UserServiceTest, CreateUserSuccess) {
    User user = user_service_->createUser("newuser", "new@example.com");
    
    EXPECT_GT(user.getId(), 0);
    EXPECT_EQ(user.getUsername(), "newuser");
    EXPECT_EQ(user.getEmail(), "new@example.com");
    EXPECT_TRUE(user_service_->getLastError().empty());
}

TEST_F(UserServiceTest, CreateUserEmptyUsername) {
    User user = user_service_->createUser("", "test@example.com");
    
    EXPECT_EQ(user.getId(), 0);
    EXPECT_EQ(user_service_->getLastError(), "Username and email are required");
}

TEST_F(UserServiceTest, CreateUserEmptyEmail) {
    User user = user_service_->createUser("testuser", "");
    
    EXPECT_EQ(user.getId(), 0);
    EXPECT_EQ(user_service_->getLastError(), "Username and email are required");
}

TEST_F(UserServiceTest, CreateUserExistingUsername) {
    User user = user_service_->createUser("existinguser", "new@example.com");
    
    EXPECT_EQ(user.getId(), 0);
    EXPECT_EQ(user_service_->getLastError(), "Username already exists");
}

TEST_F(UserServiceTest, CreateUserExistingEmail) {
    User user = user_service_->createUser("newuser", "existing@example.com");
    
    EXPECT_EQ(user.getId(), 0);
    EXPECT_EQ(user_service_->getLastError(), "Email already exists");
}

TEST_F(UserServiceTest, GetUserByIdSuccess) {
    User user = user_service_->getUserById(1);
    
    EXPECT_EQ(user.getId(), 1);
    EXPECT_EQ(user.getUsername(), "testuser");
    EXPECT_EQ(user.getEmail(), "test@example.com");
}

TEST_F(UserServiceTest, GetUserByIdNotFound) {
    User user = user_service_->getUserById(999);
    
    EXPECT_EQ(user.getId(), 0);
}

TEST_F(UserServiceTest, GetUserByUsernameSuccess) {
    User user = user_service_->getUserByUsername("testuser");
    
    EXPECT_EQ(user.getId(), 1);
    EXPECT_EQ(user.getUsername(), "testuser");
    EXPECT_EQ(user.getEmail(), "test@example.com");
}

TEST_F(UserServiceTest, GetUserByEmailSuccess) {
    User user = user_service_->getUserByEmail("test@example.com");
    
    EXPECT_EQ(user.getId(), 1);
    EXPECT_EQ(user.getUsername(), "testuser");
    EXPECT_EQ(user.getEmail(), "test@example.com");
}

TEST_F(UserServiceTest, GetAllUsers) {
    std::vector<User> users = user_service_->getAllUsers();
    
    EXPECT_EQ(users.size(), 2);
    EXPECT_EQ(users[0].getUsername(), "user1");
    EXPECT_EQ(users[1].getUsername(), "user2");
}

TEST_F(UserServiceTest, UpdateUserSuccess) {
    User user("updateduser", "updated@example.com");
    user.setId(1);
    
    bool result = user_service_->updateUser(user);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(user_service_->getLastError().empty());
}

TEST_F(UserServiceTest, UpdateUserInvalidId) {
    User user("updateduser", "updated@example.com");
    user.setId(0); // Invalid ID
    
    bool result = user_service_->updateUser(user);
    
    EXPECT_FALSE(result);
    EXPECT_EQ(user_service_->getLastError(), "Invalid user ID");
}

TEST_F(UserServiceTest, UpdateUserNotFound) {
    User user("updateduser", "updated@example.com");
    user.setId(999); // Non-existent ID
    
    bool result = user_service_->updateUser(user);
    
    EXPECT_FALSE(result);
    EXPECT_EQ(user_service_->getLastError(), "User not found");
}

TEST_F(UserServiceTest, DeleteUserSuccess) {
    bool result = user_service_->deleteUser(1);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(user_service_->getLastError().empty());
}

TEST_F(UserServiceTest, DeleteUserInvalidId) {
    bool result = user_service_->deleteUser(0);
    
    EXPECT_FALSE(result);
    EXPECT_EQ(user_service_->getLastError(), "Invalid user ID");
}

TEST_F(UserServiceTest, DeleteUserNotFound) {
    bool result = user_service_->deleteUser(999);
    
    EXPECT_FALSE(result);
    EXPECT_EQ(user_service_->getLastError(), "User not found");
}

TEST_F(UserServiceTest, IsUsernameAvailable) {
    EXPECT_TRUE(user_service_->isUsernameAvailable("newuser"));
    EXPECT_FALSE(user_service_->isUsernameAvailable("existinguser"));
}

TEST_F(UserServiceTest, IsEmailAvailable) {
    EXPECT_TRUE(user_service_->isEmailAvailable("new@example.com"));
    EXPECT_FALSE(user_service_->isEmailAvailable("existing@example.com"));
}

TEST_F(UserServiceTest, SearchUsers) {
    std::vector<User> results = user_service_->searchUsers("test");
    
    EXPECT_GT(results.size(), 0);
    EXPECT_EQ(results[0].getUsername(), "testuser");
}

TEST_F(UserServiceTest, SearchUsersByUsername) {
    std::vector<User> results = user_service_->searchUsersByUsername("test");
    
    EXPECT_GT(results.size(), 0);
    EXPECT_EQ(results[0].getUsername(), "testuser");
}

TEST_F(UserServiceTest, SearchUsersByEmail) {
    std::vector<User> results = user_service_->searchUsersByEmail("test");
    
    EXPECT_GT(results.size(), 0);
    EXPECT_EQ(results[0].getEmail(), "test@example.com");
}

TEST_F(UserServiceTest, GetUserCount) {
    int count = user_service_->getUserCount();
    
    EXPECT_EQ(count, 2);
}

TEST_F(UserServiceTest, GetUserStatistics) {
    std::string stats = user_service_->getUserStatistics();
    
    EXPECT_NE(stats.find("Total users: 2"), std::string::npos);
}

TEST_F(UserServiceTest, ValidateUser) {
    User valid_user("validuser", "valid@example.com");
    valid_user.setId(1);
    
    EXPECT_TRUE(user_service_->validateUser(valid_user));
    
    User invalid_user("", "invalid@example.com");
    EXPECT_FALSE(user_service_->validateUser(invalid_user));
}

TEST_F(UserServiceTest, SanitizeInput) {
    // This would test the private sanitizeInput method if it were public
    // For now, we test it indirectly through createUser
    
    User user = user_service_->createUser("  user with spaces  ", "test@example.com");
    
    // The actual sanitization behavior would depend on the implementation
    // This test just ensures the method doesn't crash
    EXPECT_TRUE(user_service_->getLastError().empty() || 
                user_service_->getLastError() == "Username already exists");
}

// Cache integration tests
TEST_F(UserServiceTest, CacheIntegration) {
    // Test that cache service is properly integrated
    EXPECT_TRUE(user_service_->isCacheEnabled());
    
    // Create a user (should be cached)
    User user = user_service_->createUser("cachetest", "cache@example.com");
    EXPECT_GT(user.getId(), 0);
    
    // Get the same user (should hit cache)
    User cached_user = user_service_->getUserById(user.getId());
    EXPECT_EQ(cached_user.getId(), user.getId());
    EXPECT_EQ(cached_user.getUsername(), user.getUsername());
    
    // Update user (should invalidate cache)
    cached_user.setEmail("updated@example.com");
    bool update_result = user_service_->updateUser(cached_user);
    EXPECT_TRUE(update_result);
    
    // Delete user (should invalidate cache)
    bool delete_result = user_service_->deleteUser(user.getId());
    EXPECT_TRUE(delete_result);
}

TEST_F(UserServiceTest, CacheInvalidation) {
    // Test cache invalidation methods
    user_service_->invalidateUserCache(1);
    user_service_->invalidateUserCache("testuser");
    user_service_->invalidateAllUserCache();
    
    // These operations should complete without errors
    EXPECT_TRUE(user_service_->getLastError().empty());
}