cmake_minimum_required(VERSION 3.10)
project(DatabaseArchitectureDemo)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTS "Build unit tests" ON)

# Find MySQL
find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQL REQUIRED mysqlclient)

# Find or fetch spdlog
find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
    message(STATUS "spdlog not found, fetching from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.12.0
    )
    FetchContent_MakeAvailable(spdlog)
endif()

# Find or fetch Google Test
if(BUILD_TESTS)
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        message(STATUS "Google Test not found, fetching from GitHub...")
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
        )
        FetchContent_MakeAvailable(googletest)
    endif()
    enable_testing()
endif()

# Include directories
include_directories(include)

# Common source files (excluding main.cpp for library)
set(COMMON_SOURCES
    src/database/database_config.cpp
    src/database/database_connection.cpp
    src/database/database_connection_pool.cpp
    src/utils/logger.cpp
    src/utils/config_manager.cpp
    src/cache/cache_manager.cpp
    src/cache/cache_service.cpp
    src/api/http_server.cpp
    src/api/rest_api_controller.cpp
    src/models/user.cpp
    src/models/product.cpp
    src/models/order.cpp
    src/repositories/user_repository.cpp
    src/repositories/product_repository.cpp
    src/repositories/order_repository.cpp
    src/services/user_service.cpp
    src/services/product_service.cpp
    src/services/order_service.cpp
    src/services/application_service.cpp
    src/controllers/user_controller.cpp
    src/controllers/product_controller.cpp
    src/controllers/order_controller.cpp
)

# Main executable
add_executable(${PROJECT_NAME} main.cpp ${COMMON_SOURCES})

# API Server executable
add_executable(${PROJECT_NAME}_api api_server.cpp ${COMMON_SOURCES})

# Link libraries for main executable
target_link_libraries(${PROJECT_NAME}
    ${MYSQL_LIBRARIES}
    spdlog::spdlog
)

# Link libraries for API server
target_link_libraries(${PROJECT_NAME}_api
    ${MYSQL_LIBRARIES}
    spdlog::spdlog
)

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE ${MYSQL_CFLAGS_OTHER})

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(${PROJECT_NAME}_api PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Build tests if enabled
if(BUILD_TESTS)
    # Create test library
    add_library(${PROJECT_NAME}_lib STATIC ${COMMON_SOURCES})
    target_link_libraries(${PROJECT_NAME}_lib
        ${MYSQL_LIBRARIES}
        spdlog::spdlog
    )
    target_compile_options(${PROJECT_NAME}_lib PRIVATE ${MYSQL_CFLAGS_OTHER})
    
    # Test executable
    add_executable(${PROJECT_NAME}_tests
        tests/test_main.cpp
        tests/test_database_config.cpp
        tests/test_cache_manager.cpp
        tests/test_user_service.cpp
    )
    
    target_link_libraries(${PROJECT_NAME}_tests
        ${PROJECT_NAME}_lib
        GTest::gtest
        GTest::gtest_main
        spdlog::spdlog
    )
    
    # Add tests
    add_test(NAME ${PROJECT_NAME}_tests COMMAND ${PROJECT_NAME}_tests)
    
    # Set test output directory
    set_target_properties(${PROJECT_NAME}_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    )
endif()

# Print configuration info
message(STATUS "MySQL libraries: ${MYSQL_LIBRARIES}")
message(STATUS "MySQL include directories: ${MYSQL_INCLUDE_DIRS}")
message(STATUS "MySQL compile flags: ${MYSQL_CFLAGS_OTHER}")
message(STATUS "Build tests: ${BUILD_TESTS}")

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
