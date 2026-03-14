cmake_minimum_required(VERSION 3.10)
project(order_module VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/core/include
    ${CMAKE_SOURCE_DIR}/db_util/include  # db_util headers
)

# Source files
set(ORDER_MODULE_SOURCES
    src/OrderModule.cpp
)

# Create order_module library
add_library(order_module ${ORDER_MODULE_SOURCES})

# Link with core
target_link_libraries(order_module
    core
)

# Install headers
install(DIRECTORY include/
    DESTINATION include/modules/order_module
    FILES_MATCHING PATTERN "*.h"
)

# Install library
install(TARGETS order_module
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

message(STATUS "Order module configured")
