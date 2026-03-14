cmake_minimum_required(VERSION 3.10)
project(TransportAbstraction VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(TRANSPORT_SOURCES
    src/TransportBase.cpp
    src/SerialTransport.cpp
    src/SocketTransport.cpp
    src/SharedMemoryTransport.cpp
    src/CanTransport.cpp
    src/TransportFactory.cpp
)

# Header files
set(TRANSPORT_HEADERS
    include/ITransport.h
    include/TransportBase.h
    include/SerialTransport.h
    include/SocketTransport.h
    include/SharedMemoryTransport.h
    include/CanTransport.h
    include/TransportFactory.h
)

# Create static library
add_library(transport_static STATIC ${TRANSPORT_SOURCES} ${TRANSPORT_HEADERS})
target_include_directories(transport_static PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Link pthread and rt libraries
target_link_libraries(transport_static pthread rt)

# Create shared library
add_library(transport_shared SHARED ${TRANSPORT_SOURCES} ${TRANSPORT_HEADERS})
target_include_directories(transport_shared PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(transport_shared pthread rt)

# Set library output names
set_target_properties(transport_static PROPERTIES OUTPUT_NAME transport)
set_target_properties(transport_shared PROPERTIES OUTPUT_NAME transport)

# Build examples
option(BUILD_EXAMPLES "Build example programs" ON)

if(BUILD_EXAMPLES)
    add_executable(transport_example examples/main.cpp)
    target_link_libraries(transport_example transport_static)
endif()

# Installation
install(TARGETS transport_static transport_shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${TRANSPORT_HEADERS}
    DESTINATION include/transport
)

# Print configuration
message(STATUS "")
message(STATUS "Transport Abstraction Library Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "")
