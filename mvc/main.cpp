#include <iostream>
#include <memory>
#include "database/database_config.h"
#include "database/database_connection.h"
#include "database/database_connection_pool.h"
#include "utils/logger.h"
#include "repositories/user_repository.h"
#include "services/user_service.h"
#include "controllers/user_controller.h"

void printUser(const User& user)
{
    if (user.getId() > 0)
    {
        std::cout << "User: " << user.toString() << std::endl;
    }
    else
    {
        std::cout << "User not found" << std::endl;
    }
}

void printUsers(const std::vector<User>& users)
{
    if (users.empty())
    {
        std::cout << "No users found" << std::endl;
        return;
    }
    
    std::cout << "Found " << users.size() << " users:" << std::endl;
    for (const auto& user : users)
    {
        printUser(user);
    }
}

int main()
{
    std::cout << "=== C++ Database Architecture Demo ===" << std::endl;

    try
    {
        // Initialize logger system
        Logger::initialize("DatabaseApp", "logs/database_app.log", 5 * 1024 * 1024, 3, spdlog::level::debug);
        Logger::info("Application starting...");

        // Initialize configuration from file (if exists) or use defaults
        std::string config_file = "config.json";
        DatabaseConfig& config = DatabaseConfig::getInstance();
        config.initialize(config_file);
        
        // Initialize logger with configuration
        auto& config_manager = ConfigManager::getInstance();
        auto log_config = config_manager.getLogConfig();
        
        spdlog::level::level_enum log_level;
        std::string level_str = log_config.level;
        std::transform(level_str.begin(), level_str.end(), level_str.begin(), ::tolower);
        
        if (level_str == "trace") log_level = spdlog::level::trace;
        else if (level_str == "debug") log_level = spdlog::level::debug;
        else if (level_str == "info") log_level = spdlog::level::info;
        else if (level_str == "warn") log_level = spdlog::level::warn;
        else if (level_str == "error") log_level = spdlog::level::err;
        else if (level_str == "critical") log_level = spdlog::level::critical;
        else log_level = spdlog::level::info;
        
        Logger::initialize("DatabaseApp", log_config.file, log_config.max_file_size, 
                          log_config.max_files, log_level);

        Logger::info("Database configuration loaded");

        // Initialize connection pool using configuration
        auto& pool = DatabaseConnectionPool::getInstance();
        if (!pool.initialize(0, 0, std::chrono::seconds(0))) // 0 means use config values
        {
            Logger::error("Failed to initialize connection pool");
            return 1;
        }

        Logger::info("Connection pool initialized successfully");
        Logger::info("Pool stats - Total: {}, Available: {}, Active: {}",
                    pool.getTotalConnections(),
                    pool.getAvailableConnections(),
                    pool.getActiveConnections());

        // Get a connection from the pool
        auto pooled_conn = pool.getConnection();
        auto connection = pooled_conn.get();

        Logger::info("Got connection from pool");

        // Create repository
        auto user_repo = std::make_shared<UserRepository>(connection);
        Logger::info("User repository created");

        // Create service
        auto user_service = std::make_shared<UserService>(user_repo);
        Logger::info("User service created");

        // Create controller
        auto user_controller = std::make_shared<UserController>(user_service);
        Logger::info("User controller created");
        
        std::cout << "\n=== Testing User Operations ===" << std::endl;
        Logger::info("Starting user operations tests");

        // Test creating users
        std::cout << "\n1. Creating users..." << std::endl;
        Logger::info("Test 1: Creating users");

        auto result1 = user_controller->createUser("john_doe", "john@example.com");
        if (result1.success)
        {
            std::cout << "Created user: " << result1.user.toString() << std::endl;
            Logger::info("Successfully created user: {}", result1.user.getUsername());
        }
        else
        {
            std::cout << "Failed to create user: " << result1.error_message << std::endl;
            Logger::warn("Failed to create user: {}", result1.error_message);
        }

        auto result2 = user_controller->createUser("jane_smith", "jane@example.com");
        if (result2.success)
        {
            std::cout << "Created user: " << result2.user.toString() << std::endl;
            Logger::info("Successfully created user: {}", result2.user.getUsername());
        }
        else
        {
            std::cout << "Failed to create user: " << result2.error_message << std::endl;
            Logger::warn("Failed to create user: {}", result2.error_message);
        }
        
        // Test getting users
        std::cout << "\n2. Getting users..." << std::endl;
        Logger::info("Test 2: Getting users");

        if (result1.success)
        {
            User user = user_controller->getUserById(result1.user.getId());
            std::cout << "Retrieved user by ID: ";
            printUser(user);
            Logger::info("Retrieved user by ID: {}", result1.user.getId());
        }

        User user_by_username = user_controller->getUserByUsername("jane_smith");
        std::cout << "Retrieved user by username: ";
        printUser(user_by_username);
        Logger::info("Retrieved user by username: jane_smith");

        // Test getting all users
        std::cout << "\n3. Getting all users..." << std::endl;
        Logger::info("Test 3: Getting all users");
        auto all_users = user_controller->getAllUsers();
        printUsers(all_users);
        Logger::info("Retrieved {} users", all_users.size());

        // Test search functionality
        std::cout << "\n4. Testing search functionality..." << std::endl;
        Logger::info("Test 4: Testing search functionality");

        auto search_results = user_controller->searchUsers("john");
        std::cout << "Search results for 'john':" << std::endl;
        printUsers(search_results);
        Logger::info("Search for 'john' returned {} results", search_results.size());

        auto username_search = user_controller->searchUsersByUsername("jane");
        std::cout << "Username search results for 'jane':" << std::endl;
        printUsers(username_search);
        Logger::info("Username search for 'jane' returned {} results", username_search.size());
        
        // Test validation
        std::cout << "\n5. Testing validation..." << std::endl;
        Logger::info("Test 5: Testing validation");

        bool username_available = user_controller->isUsernameAvailable("new_user");
        std::cout << "Username 'new_user' available: " << (username_available ? "Yes" : "No") << std::endl;
        Logger::debug("Username 'new_user' availability: {}", username_available);

        bool email_available = user_controller->isEmailAvailable("new@example.com");
        std::cout << "Email 'new@example.com' available: " << (email_available ? "Yes" : "No") << std::endl;
        Logger::debug("Email 'new@example.com' availability: {}", email_available);

        // Test updating user
        std::cout << "\n6. Testing user update..." << std::endl;
        Logger::info("Test 6: Testing user update");

        if (result1.success)
        {
            User user_to_update = result1.user;
            user_to_update.setEmail("john.updated@example.com");

            auto update_result = user_controller->updateUser(user_to_update);
            if (update_result.success)
            {
                std::cout << "User updated successfully" << std::endl;
                Logger::info("User {} updated successfully", user_to_update.getId());

                // Verify update
                User updated_user = user_controller->getUserById(user_to_update.getId());
                std::cout << "Updated user: ";
                printUser(updated_user);
            }
            else
            {
                std::cout << "Failed to update user: " << update_result.error_message << std::endl;
                Logger::error("Failed to update user: {}", update_result.error_message);
            }
        }

        // Test statistics
        std::cout << "\n7. User statistics..." << std::endl;
        Logger::info("Test 7: User statistics");
        std::string stats = user_controller->getUserStatistics();
        std::cout << stats << std::endl;

        // Test error handling
        std::cout << "\n8. Testing error handling..." << std::endl;
        Logger::info("Test 8: Testing error handling");

        // Try to create user with existing username
        auto duplicate_result = user_controller->createUser("john_doe", "duplicate@example.com");
        if (!duplicate_result.success)
        {
            std::cout << "Correctly prevented duplicate username: " << duplicate_result.error_message << std::endl;
            Logger::info("Correctly prevented duplicate username");
        }

        // Try to get non-existent user
        User non_existent = user_controller->getUserById(99999);
        std::cout << "Non-existent user result: ";
        printUser(non_existent);
        Logger::debug("Attempted to get non-existent user with ID 99999");

        // Show final connection pool statistics
        Logger::info("Final pool stats - Total: {}, Available: {}, Active: {}",
                    pool.getTotalConnections(),
                    pool.getAvailableConnections(),
                    pool.getActiveConnections());

        std::cout << "\n=== Demo completed successfully ===" << std::endl;
        Logger::info("Demo completed successfully");
        
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        Logger::critical("Fatal exception occurred: {}", e.what());
        Logger::flush();
        Logger::shutdown();
        return 1;
    }

    // Cleanup
    Logger::info("Shutting down application...");
    Logger::flush();
    Logger::shutdown();

    return 0;
}
