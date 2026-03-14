cmake_minimum_required(VERSION 3.16)
project(tcp_network_lib VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Find required packages
find_package(Threads REQUIRED)

# Platform-specific settings
if(UNIX AND NOT APPLE)
    # Linux specific settings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(PLATFORM_LIBS pthread rt)
elseif(APPLE)
    # macOS specific settings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(PLATFORM_LIBS pthread)
elseif(WIN32)
    # Windows specific settings
    set(PLATFORM_LIBS ws2_32 iphlpapi)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR})

# Add subdirectories
add_subdirectory(src)
add_subdirectory(examples)

# Install configuration
include(GNUInstallDirs)

# Install headers
install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/
        FILES_MATCHING PATTERN "*.h")

# Install library
install(TARGETS tcp_network_lib
        EXPORT tcp_network_libTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tcp_network)

# Install example binaries
# install(TARGETS enhanced_tcp_example
#         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Export targets
install(EXPORT tcp_network_libTargets
        FILE tcp_network_libTargets.cmake
        NAMESPACE tcp_network_lib::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tcp_network_lib)

# Create config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/tcp_network_libConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/tcp_network_libConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tcp_network_lib
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/tcp_network_libConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/tcp_network_libConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/tcp_network_libConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tcp_network_lib
)
