cmake_minimum_required(VERSION 3.10)
project(db_util VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find MySQL Connector/C++
find_library(MYSQL_CPPCONN_LIB mysqlcppconn)
if(NOT MYSQL_CPPCONN_LIB)
    message(FATAL_ERROR "MySQL Connector/C++ library not found")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(DB_UTIL_SOURCES
    src/DatabaseManager.cpp
    src/TableManager.cpp
)

# Create db_util library
add_library(db_util ${DB_UTIL_SOURCES})

# Link with MySQL Connector/C++
target_link_libraries(db_util ${MYSQL_CPPCONN_LIB})

# Export include directories for other modules
target_include_directories(db_util PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/db_util>
)

# Build examples (optional)
option(BUILD_DB_UTIL_EXAMPLES "Build db_util example programs" ON)
if(BUILD_DB_UTIL_EXAMPLES)
    add_executable(easy_usage_example examples/easy_usage_example.cpp)
    target_link_libraries(easy_usage_example db_util ${MYSQL_CPPCONN_LIB})

    add_executable(simple_db_example examples/simple_db_example.cpp)
    target_link_libraries(simple_db_example db_util ${MYSQL_CPPCONN_LIB})

    add_executable(simple_table_example examples/simple_table_example.cpp)
    target_link_libraries(simple_table_example db_util ${MYSQL_CPPCONN_LIB})

    message(STATUS "Building db_util examples: enabled")
endif()

# Build tests (optional)
option(BUILD_DB_UTIL_TESTS "Build db_util tests" ON)
if(BUILD_DB_UTIL_TESTS)
    add_executable(debug_test tests/debug_test.cpp)
    target_link_libraries(debug_test db_util ${MYSQL_CPPCONN_LIB})

    message(STATUS "Building db_util tests: enabled")
endif()

# Install headers
install(DIRECTORY include/
    DESTINATION include/db_util
    FILES_MATCHING PATTERN "*.h"
)

# Install library
install(TARGETS db_util
    EXPORT db_utilTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

message(STATUS "db_util infrastructure layer configured")
