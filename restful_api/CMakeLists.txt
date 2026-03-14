cmake_minimum_required(VERSION 3.10.0)
project(test VERSION 0.1.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Original test executable
add_executable(test main.cpp)

# HTTP API server executable
add_executable(server server.cpp)
target_link_libraries(server pthread)

# HTTP API client executable  
add_executable(client client.cpp)
target_link_libraries(client pthread)

# Async HTTP API client executable
add_executable(async_client async_client.cpp)
target_link_libraries(async_client pthread)

