cmake_minimum_required(VERSION 3.6)
project(device_connection_checker)

# Compiler settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../uos_base/include
)

# Source files
set(DEVICE_CHECKER_SOURCES
    src/device_connection_checker.cc
)

# Header files
set(DEVICE_CHECKER_HEADERS
    include/device_connection_checker.h
    include/device_connection_checker_defaults.h
    include/device_connection_checker_interfaces.h
    include/device_connection_checker_types.h
)

# Create static library
add_library(device_connection_checker STATIC ${DEVICE_CHECKER_SOURCES})

# Link dependencies
target_link_libraries(device_connection_checker
    uos_base
)

# Compiler flags
target_compile_options(device_connection_checker PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
)

# Install headers
install(FILES ${DEVICE_CHECKER_HEADERS}
        DESTINATION include/device_connection_checker)

# Install library
install(TARGETS device_connection_checker
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

# Install configuration example
install(FILES etc/device_connection_check.json
        DESTINATION etc/device_connection_checker)

# Install documentation
install(FILES
        README.md
        docs/example_usage.cc
        docs/device_implementation_example.cc
        DESTINATION share/doc/device_connection_checker)

# Optional: Build example
option(BUILD_DEVICE_CHECKER_EXAMPLE "Build device connection checker example" OFF)

if(BUILD_DEVICE_CHECKER_EXAMPLE)
    add_executable(device_checker_example
        docs/example_usage.cc
    )

    target_link_libraries(device_checker_example
        device_connection_checker
        uos_base
    )

    install(TARGETS device_checker_example
            RUNTIME DESTINATION bin)
endif()

# Optional: Build unit tests
option(BUILD_DEVICE_CHECKER_TESTS "Build device connection checker unit tests" ON)

if(BUILD_DEVICE_CHECKER_TESTS)
    # Find GTest
    find_package(GTest QUIET)

    if(GTEST_FOUND)
        enable_testing()

        # Unit test executable
        add_executable(device_connection_checker_ut
            test/device_connection_checker_ut.cc
            src/device_connection_checker.cc
        )

        target_include_directories(device_connection_checker_ut PRIVATE
            ${GTEST_INCLUDE_DIRS}
        )

        target_link_libraries(device_connection_checker_ut
            uos_base
            ${GTEST_BOTH_LIBRARIES}
            pthread
            rt
        )

        target_compile_options(device_connection_checker_ut PRIVATE
            -std=c++11
        )

        # Add test to CTest
        add_test(NAME device_connection_checker_ut
                 COMMAND device_connection_checker_ut)

        message(STATUS "  Build tests: ${BUILD_DEVICE_CHECKER_TESTS}")
    else()
        message(WARNING "GTest not found, skipping tests")
        set(BUILD_DEVICE_CHECKER_TESTS OFF)
    endif()
endif()

# Print build information
message(STATUS "Device Connection Checker Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build example: ${BUILD_DEVICE_CHECKER_EXAMPLE}")
message(STATUS "  Build tests: ${BUILD_DEVICE_CHECKER_TESTS}")
