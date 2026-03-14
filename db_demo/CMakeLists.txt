cmake_minimum_required(VERSION 3.10)
project(db_util_layered VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# Find MySQL Connector/C++
find_library(MYSQL_CPPCONN_LIB mysqlcppconn)
if(NOT MYSQL_CPPCONN_LIB)
    message(FATAL_ERROR "MySQL Connector/C++ library not found. Please install: sudo apt-get install libmysqlcppconn-dev")
endif()
message(STATUS "Found MySQL Connector/C++: ${MYSQL_CPPCONN_LIB}")

# Add subdirectories for layered architecture
add_subdirectory(db_util)  # Infrastructure layer
add_subdirectory(core)     # DAO layer
add_subdirectory(modules/user_module)   # Business logic
add_subdirectory(modules/order_module)  # Business logic
add_subdirectory(app)      # Application layer

# Generate package config
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/db_utilConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/db_utilConfig.cmake"
    INSTALL_DESTINATION lib/cmake/db_util
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/db_utilConfig.cmake"
    DESTINATION lib/cmake/db_util
)

# Summary
message(STATUS "")
message(STATUS "═══════════════════════════════════════════════════════════")
message(STATUS "  Project: ${PROJECT_NAME}")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "═══════════════════════════════════════════════════════════")
message(STATUS "  Architecture Layers:")
message(STATUS "    ├─ db_util       (Infrastructure)")
message(STATUS "    ├─ core          (DAO + Entities + Service)")
message(STATUS "    ├─ modules       (Business Logic)")
message(STATUS "    │   ├─ user_module")
message(STATUS "    │   └─ order_module")
message(STATUS "    └─ app           (Application Entry)")
message(STATUS "═══════════════════════════════════════════════════════════")
message(STATUS "")
