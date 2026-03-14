cmake_minimum_required(VERSION 3.10)
project(user_module VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/core/include
    ${CMAKE_SOURCE_DIR}/db_util/include  # db_util headers
)

# Source files
set(USER_MODULE_SOURCES
    src/UserModule.cpp
)

# Create user_module library
add_library(user_module ${USER_MODULE_SOURCES})

# Link with core
target_link_libraries(user_module
    core
)

# Install headers
install(DIRECTORY include/
    DESTINATION include/modules/user_module
    FILES_MATCHING PATTERN "*.h"
)

# Install library
install(TARGETS user_module
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

message(STATUS "User module configured")
