cmake_minimum_required(VERSION 3.10)
project(network_lib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/tcp_socket.cpp
    src/udp_socket.cpp
)

# Header files
set(HEADERS
    include/network_lib/tcp_socket.h
    include/network_lib/udp_socket.h
    include/network_lib/protocol.h
)

# Create library
add_library(network_lib STATIC ${SOURCES} ${HEADERS})

# Find pthread library
find_package(Threads REQUIRED)
target_link_libraries(network_lib Threads::Threads)

# Examples
add_executable(tcp_example
    examples/tcp_client_server.cpp
    examples/simple_protocol.cpp
)
target_link_libraries(tcp_example network_lib Threads::Threads)
target_include_directories(tcp_example PRIVATE examples)

add_executable(udp_example
    examples/udp_client_server.cpp
    examples/simple_protocol.cpp
)
target_link_libraries(udp_example network_lib Threads::Threads)
target_include_directories(udp_example PRIVATE examples)

add_executable(protocol_example
    examples/protocol_example.cpp
    examples/simple_protocol.cpp
)
target_link_libraries(protocol_example network_lib)
target_include_directories(protocol_example PRIVATE examples)

# Installation
install(TARGETS network_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/network_lib
    DESTINATION include
)

