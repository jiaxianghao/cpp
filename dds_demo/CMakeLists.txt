cmake_minimum_required(VERSION 3.16)
project(DDSWrapper VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find FastDDS
find_package(fastdds REQUIRED)
find_package(fastcdr REQUIRED)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# Copy config directory to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
message(STATUS "Config files copied to ${CMAKE_CURRENT_BINARY_DIR}/config")

# Add subdirectories
add_subdirectory(src)
add_subdirectory(idl)
add_subdirectory(examples)
add_subdirectory(tools)

# Installation
install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY config/
    DESTINATION config
    FILES_MATCHING PATTERN "*.json" PATTERN "*.xml"
)
