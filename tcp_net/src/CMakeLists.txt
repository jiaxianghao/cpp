# Create library target
add_library(tcp_network_lib
    tcp_communicator.cc
    tcp_communicator_config.cc
    heartbeat_strategy.cc
    message_boundary_detector.cc
)

# Set library properties
set_target_properties(tcp_network_lib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER include/tcp_network/tcp_communicator.h
)

# Link libraries
target_link_libraries(tcp_network_lib
    Threads::Threads
    ${PLATFORM_LIBS}
)

# Set include directories for the library
target_include_directories(tcp_network_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tcp_network_lib>
    PRIVATE
        ${CMAKE_SOURCE_DIR}
)

# Compiler definitions
target_compile_definitions(tcp_network_lib
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)

# Set C++ standard for the target
set_target_properties(tcp_network_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
