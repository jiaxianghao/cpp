cmake_minimum_required(VERSION 3.10)
project(Examples)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find MySQL
find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQL REQUIRED mysqlclient)

# Include directories
include_directories(../include)

# Source files for the main library
set(LIB_SOURCES
    ../src/database/database_config.cpp
    ../src/database/database_connection.cpp
    ../src/models/user.cpp
    ../src/models/product.cpp
    ../src/models/order.cpp
    ../src/repositories/user_repository.cpp
    ../src/repositories/product_repository.cpp
    ../src/repositories/order_repository.cpp
    ../src/services/user_service.cpp
    ../src/services/product_service.cpp
    ../src/services/order_service.cpp
    ../src/services/application_service.cpp
    ../src/controllers/user_controller.cpp
    ../src/controllers/product_controller.cpp
    ../src/controllers/order_controller.cpp
)

# Create shared library
add_library(database_arch SHARED ${LIB_SOURCES})
target_link_libraries(database_arch ${MYSQL_LIBRARIES})
target_compile_options(database_arch PRIVATE ${MYSQL_CFLAGS_OTHER})

# Simple usage example
add_executable(simple_usage simple_usage.cpp)
target_link_libraries(simple_usage database_arch)

# Business application example
add_executable(business_application business_application.cpp)
target_link_libraries(business_application database_arch)

# Web API example
add_executable(web_api_example web_api_example.cpp)
target_link_libraries(web_api_example database_arch)

# Multi entity example
add_executable(multi_entity_example multi_entity_example.cpp)
target_link_libraries(multi_entity_example database_arch)

# Complete architecture example
add_executable(complete_architecture_example complete_architecture_example.cpp)
target_link_libraries(complete_architecture_example database_arch)

# Set output directory
set_target_properties(simple_usage business_application web_api_example multi_entity_example complete_architecture_example
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
