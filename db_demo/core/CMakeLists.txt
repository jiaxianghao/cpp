cmake_minimum_required(VERSION 3.10)
project(core VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find MySQL Connector/C++
find_library(MYSQL_CPPCONN_LIB mysqlcppconn)
if(NOT MYSQL_CPPCONN_LIB)
    message(FATAL_ERROR "MySQL Connector/C++ library not found")
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/db_util/include  # db_util headers
)

# Source files
set(CORE_SOURCES
    src/entities/User.cpp
    src/entities/Order.cpp
    src/entities/Product.cpp
    src/dao/UserDAO.cpp
    src/dao/OrderDAO.cpp
    src/dao/ProductDAO.cpp
    src/DatabaseService.cpp
)

# Create core library
add_library(core ${CORE_SOURCES})

# Link with db_util (built in parent project)
target_link_libraries(core
    db_util
    ${MYSQL_CPPCONN_LIB}
)

# Install headers
install(DIRECTORY include/
    DESTINATION include/core
    FILES_MATCHING PATTERN "*.h"
)

# Install library
install(TARGETS core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

message(STATUS "Core module configured")
