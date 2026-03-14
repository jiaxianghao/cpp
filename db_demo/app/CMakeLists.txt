cmake_minimum_required(VERSION 3.10)
project(app VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/core/include
    ${CMAKE_SOURCE_DIR}/modules/user_module/include
    ${CMAKE_SOURCE_DIR}/modules/order_module/include
    ${CMAKE_SOURCE_DIR}/db_util/include  # db_util headers
)

# Create executable
add_executable(layered_db_app main.cpp)

# Link with all modules
target_link_libraries(layered_db_app
    user_module
    order_module
    core
    db_util
    ${MYSQL_CPPCONN_LIB}
)

# Install executable
install(TARGETS layered_db_app
    RUNTIME DESTINATION bin
)

message(STATUS "Application configured")
