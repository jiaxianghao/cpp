#ifndef TEST_MAIN_H
#define TEST_MAIN_H

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include "utils/logger.h"

// Test main function
int main(int argc, char **argv) {
    // Initialize logging for tests
    Logger::initialize("DatabaseAppTests", "logs/test.log", 1024 * 1024, 1, spdlog::level::debug);
    
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // Run tests
    int result = RUN_ALL_TESTS();
    
    // Cleanup
    Logger::shutdown();
    
    return result;
}

#endif // TEST_MAIN_H