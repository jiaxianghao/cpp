cmake_minimum_required(VERSION 3.14)

project(thread_pool LANGUAGES CXX)

find_package(Threads REQUIRED)

add_library(thread_pool INTERFACE)
target_include_directories(thread_pool INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(thread_pool INTERFACE cxx_std_17)
target_link_libraries(thread_pool INTERFACE Threads::Threads)

add_executable(thread_pool_example src/main.cpp)
target_link_libraries(thread_pool_example PRIVATE thread_pool)

enable_testing()

add_executable(thread_pool_tests tests/thread_pool_tests.cpp)
target_link_libraries(thread_pool_tests PRIVATE thread_pool)
add_test(NAME thread_pool_tests COMMAND thread_pool_tests)
