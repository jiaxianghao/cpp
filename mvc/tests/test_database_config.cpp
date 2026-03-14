#include <gtest/gtest.h>
#include "database/database_config.h"
#include "utils/config_manager.h"
#include <filesystem>

class DatabaseConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test config file
        test_config_file_ = "test_config.json";
        std::ofstream config_file(test_config_file_);
        config_file << "{\n";
        config_file << "  \"database\": {\n";
        config_file << "    \"host\": \"test_host\",\n";
        config_file << "    \"port\": 3307,\n";
        config_file << "    \"user\": \"test_user\",\n";
        config_file << "    \"password\": \"test_password\",\n";
        config_file << "    \"database\": \"test_db\",\n";
        config_file << "    \"max_connections\": 15,\n";
        config_file << "    \"min_connections\": 3,\n";
        config_file << "    \"connection_timeout\": 45,\n";
        config_file << "    \"auto_reconnect\": false\n";
        config_file << "  }\n";
        config_file << "}\n";
        config_file.close();
    }

    void TearDown() override {
        // Clean up test config file
        if (std::filesystem::exists(test_config_file_)) {
            std::filesystem::remove(test_config_file_);
        }
    }

    std::string test_config_file_;
};

TEST_F(DatabaseConfigTest, DefaultConfiguration) {
    auto& config = DatabaseConfig::getInstance();
    
    // Test default values
    EXPECT_EQ(config.getHost(), "localhost");
    EXPECT_EQ(config.getPort(), 3306);
    EXPECT_EQ(config.getUser(), "root");
    EXPECT_EQ(config.getPassword(), "password");
    EXPECT_EQ(config.getDatabase(), "myapp");
    EXPECT_EQ(config.getMaxConnections(), 10);
    EXPECT_EQ(config.getMinConnections(), 5);
    EXPECT_EQ(config.getConnectionTimeout(), 30);
    EXPECT_TRUE(config.getAutoReconnect());
}

TEST_F(DatabaseConfigTest, ConfigurationFromFile) {
    auto& config = DatabaseConfig::getInstance();
    
    // Initialize from test config file
    config.initialize(test_config_file_);
    
    // Verify loaded configuration
    EXPECT_EQ(config.getHost(), "test_host");
    EXPECT_EQ(config.getPort(), 3307);
    EXPECT_EQ(config.getUser(), "test_user");
    EXPECT_EQ(config.getPassword(), "test_password");
    EXPECT_EQ(config.getDatabase(), "test_db");
    EXPECT_EQ(config.getMaxConnections(), 15);
    EXPECT_EQ(config.getMinConnections(), 3);
    EXPECT_EQ(config.getConnectionTimeout(), 45);
    EXPECT_FALSE(config.getAutoReconnect());
}

TEST_F(DatabaseConfigTest, SettersAndGetters) {
    auto& config = DatabaseConfig::getInstance();
    
    // Test setters
    config.setHost("new_host");
    config.setPort(3308);
    config.setUser("new_user");
    config.setPassword("new_password");
    config.setDatabase("new_db");
    config.setMaxConnections(20);
    config.setMinConnections(2);
    config.setConnectionTimeout(60);
    config.setAutoReconnect(false);
    
    // Verify set values
    EXPECT_EQ(config.getHost(), "new_host");
    EXPECT_EQ(config.getPort(), 3308);
    EXPECT_EQ(config.getUser(), "new_user");
    EXPECT_EQ(config.getPassword(), "new_password");
    EXPECT_EQ(config.getDatabase(), "new_db");
    EXPECT_EQ(config.getMaxConnections(), 20);
    EXPECT_EQ(config.getMinConnections(), 2);
    EXPECT_EQ(config.getConnectionTimeout(), 60);
    EXPECT_FALSE(config.getAutoReconnect());
}

TEST_F(DatabaseConfigTest, SingletonInstance) {
    auto& config1 = DatabaseConfig::getInstance();
    auto& config2 = DatabaseConfig::getInstance();
    
    // Verify singleton behavior
    EXPECT_EQ(&config1, &config2);
}

TEST_F(DatabaseConfigTest, NonExistentConfigFile) {
    auto& config = DatabaseConfig::getInstance();
    
    // Initialize with non-existent file should use defaults
    config.initialize("non_existent_config.json");
    
    // Should fall back to default values
    EXPECT_EQ(config.getHost(), "localhost");
    EXPECT_EQ(config.getPort(), 3306);
}