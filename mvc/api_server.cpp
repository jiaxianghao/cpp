#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <csignal>

#include "api/http_server.h"
#include "api/rest_api_controller.h"
#include "database/database_config.h"
#include "database/database_connection.h"
#include "database/database_connection_pool.h"
#include "cache/cache_service.h"
#include "utils/logger.h"
#include "utils/config_manager.h"
#include "repositories/user_repository.h"
#include "services/user_service.h"
#include "controllers/user_controller.h"

// Global server pointer for signal handling
std::unique_ptr<HttpServer> g_server = nullptr;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down gracefully..." << std::endl;
    if (g_server && g_server->isRunning()) {
        g_server->stop();
    }
}

void printBanner() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════════════════════╗
║                                                                              ║
║    C++ Database Architecture REST API Server                                 ║
║                                                                              ║
║    A high-performance RESTful API with:                                    ║
║    ✓ Connection Pool Management                                           ║
║    ✓ Redis-like Caching Layer                                              ║
║    ✓ Structured Logging with spdlog                                        ║
║    ✓ Configuration Management                                               ║
║    ✓ Comprehensive Unit Testing                                            ║
║    ✓ RESTful API Endpoints                                                 ║
║                                                                              ║
╚══════════════════════════════════════════════════════════════════════════════╝
)" << std::endl;
}

void printUsage() {
    std::cout << "Usage: api_server [options]\n"
              << "Options:\n"
              << "  --config <file>     Configuration file path (default: config.json)\n"
              << "  --port <port>       Server port (default: 8080)\n"
              << "  --host <host>       Server host (default: 0.0.0.0)\n"
              << "  --log-level <level> Log level: trace, debug, info, warn, error (default: info)\n"
              << "  --help              Show this help message\n";
}

int main(int argc, char* argv[]) {
    printBanner();
    
    // Default configuration
    std::string config_file = "config.json";
    std::string host = "0.0.0.0";
    int port = 8080;
    std::string log_level = "info";
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            printUsage();
            return 0;
        } else if (arg == "--config" && i + 1 < argc) {
            config_file = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (arg == "--log-level" && i + 1 < argc) {
            log_level = argv[++i];
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            printUsage();
            return 1;
        }
    }
    
    try {
        // Initialize configuration
        std::cout << "Loading configuration from: " << config_file << std::endl;
        auto& config_manager = ConfigManager::getInstance();
        if (!config_manager.loadConfig(config_file)) {
            std::cout << "Warning: Failed to load configuration file, using defaults" << std::endl;
        }
        
        // Initialize logger
        auto log_config = config_manager.getLogConfig();
        spdlog::level::level_enum level = spdlog::level::info;
        
        if (log_level == "trace") level = spdlog::level::trace;
        else if (log_level == "debug") level = spdlog::level::debug;
        else if (log_level == "info") level = spdlog::level::info;
        else if (log_level == "warn") level = spdlog::level::warn;
        else if (log_level == "error") level = spdlog::level::err;
        else if (log_level == "critical") level = spdlog::level::critical;
        
        Logger::initialize("DatabaseAPI", log_config.file, log_config.max_file_size, 
                          log_config.max_files, level);
        
        Logger::info("=== C++ Database Architecture REST API Server Starting ===");
        Logger::info("Configuration file: {}", config_file);
        Logger::info("Server: {}:{}", host, port);
        Logger::info("Log level: {}", log_level);
        
        // Initialize database configuration
        auto& db_config = DatabaseConfig::getInstance();
        db_config.initialize(config_file);
        
        // Initialize connection pool
        Logger::info("Initializing database connection pool...");
        auto& pool = DatabaseConnectionPool::getInstance();
        if (!pool.initialize(0, 0, std::chrono::seconds(0))) { // Use config values
            Logger::error("Failed to initialize connection pool");
            return 1;
        }
        
        Logger::info("Connection pool initialized successfully");
        Logger::info("Pool stats - Total: {}, Available: {}, Active: {}",
                    pool.getTotalConnections(),
                    pool.getAvailableConnections(),
                    pool.getActiveConnections());
        
        // Initialize cache service
        Logger::info("Initializing cache service...");
        auto& cache_service = CacheService::getInstance();
        if (!cache_service.initialize(config_file)) {
            Logger::warn("Failed to initialize cache service, continuing without cache");
        } else {
            Logger::info("Cache service initialized successfully");
        }
        
        // Get database connection
        auto pooled_conn = pool.getConnection();
        auto connection = pooled_conn.get();
        
        if (!connection || !connection->isConnected()) {
            Logger::error("Failed to get database connection");
            return 1;
        }
        
        // Create controllers and services
        Logger::info("Creating application services...");
        
        // User components
        auto user_repo = std::make_shared<UserRepository>(connection);
        auto user_service = std::make_shared<UserService>(user_repo, 
                                                         std::make_shared<CacheService>());
        auto user_controller = std::make_shared<UserController>(user_service);
        
        // Create HTTP server
        Logger::info("Creating HTTP server...");
        g_server = std::make_unique<HttpServer>(host, port);
        
        // Configure server
        auto server_config = config_manager.getServerConfig();
        g_server->setThreadPoolSize(server_config.thread_pool_size);
        g_server->setRequestTimeout(server_config.request_timeout);
        g_server->enableCORS("*");
        
        // Create and configure REST API controller
        Logger::info("Configuring REST API routes...");
        auto api_controller = std::make_unique<RestApiController>(user_controller);
        api_controller->initializeRoutes(*g_server);
        
        // Add middleware for logging
        g_server->use([](const HttpRequest& request) {
            Logger::info("{} {} {}", request.method, request.path, request.version);
            return HttpResponse::ok(); // Continue to next middleware/handler
        });
        
        // Add error handling middleware
        g_server->setErrorHandler([](const HttpRequest& request) {
            Logger::error("Error processing request: {} {}", request.method, request.path);
            return HttpResponse::internalServerError("Internal server error");
        });
        
        // Set up signal handlers for graceful shutdown
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);
        
        // Start the server
        Logger::info("Starting HTTP server on {}:{}...", host, port);
        if (!g_server->start()) {
            Logger::error("Failed to start HTTP server");
            return 1;
        }
        
        Logger::info("HTTP server started successfully");
        Logger::info("API endpoints available at: http://{}:{}/api/", host, port);
        
        // Print available endpoints
        std::cout << "\nAvailable API Endpoints:" << std::endl;
        std::cout << "═══════════════════════════════════════════════════════════════" << std::endl;
        std::cout << "Health Check:" << std::endl;
        std::cout << "  GET  /api/health          - Health check" << std::endl;
        std::cout << "  GET  /api/info            - API information" << std::endl;
        std::cout << std::endl;
        std::cout << "User Management:" << std::endl;
        std::cout << "  GET    /api/users         - Get all users" << std::endl;
        std::cout << "  GET    /api/users/:id     - Get user by ID" << std::endl;
        std::cout << "  POST   /api/users         - Create new user" << std::endl;
        std::cout << "  PUT    /api/users/:id     - Update user" << std::endl;
        std::cout << "  DELETE /api/users/:id     - Delete user" << std::endl;
        std::cout << "  GET    /api/users/search?q=<query> - Search users" << std::endl;
        std::cout << "  GET    /api/users/check-username/:username - Check username availability" << std::endl;
        std::cout << "  GET    /api/users/check-email/:email - Check email availability" << std::endl;
        std::cout << "  GET    /api/users/stats   - Get user statistics" << std::endl;
        std::cout << std::endl;
        std::cout << "Examples:" << std::endl;
        std::cout << "  curl http://" << host << ":" << port << "/api/health" << std::endl;
        std::cout << "  curl http://" << host << ":" << port << "/api/users" << std::endl;
        std::cout << "  curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"john\",\"email\":\"john@example.com\"}' http://" << host << ":" << port << "/api/users" << std::endl;
        std::cout << "═══════════════════════════════════════════════════════════════" << std::endl;
        
        // Keep the server running
        std::cout << "\nServer is running. Press Ctrl+C to stop.\n" << std::endl;
        
        while (g_server->isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
    } catch (const std::exception& e) {
        Logger::critical("Fatal error: {}", e.what());
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    // Cleanup
    Logger::info("Shutting down server...");
    
    if (g_server) {
        g_server->stop();
    }
    
    Logger::info("Server shutdown complete");
    Logger::shutdown();
    
    return 0;
}