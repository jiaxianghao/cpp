cmake_minimum_required(VERSION 3.10)
project(uos_rmap_mock_test)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 启用警告和调试信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g")

# 查找 Google Test 和 Google Mock
find_package(GTest REQUIRED)

# 包含头文件路径
include_directories(${GTEST_INCLUDE_DIRS})

# 添加源文件
set(SOURCES
    mock_lib/uos_rmap_mock.h
    ut/uos_rmap_mock_test.cc
)

# 添加测试目标
add_executable(uos_rmap_mock_test ${SOURCES})

# 链接 Google Test 和 Google Mock 库
target_link_libraries(uos_rmap_mock_test ${GTEST_LIBRARIES} pthread)