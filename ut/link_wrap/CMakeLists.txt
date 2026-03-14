cmake_minimum_required(VERSION 3.10)
project(wrap_demo)

set(CMAKE_C_STANDARD 99)

# Create a static library with the original functions
add_library(original_lib STATIC
    original_lib.c
)

# Create the main executable
add_executable(wrap_demo main.c)

# Link original_lib to main executable and apply --wrap linker option
# This tells the linker to redirect calls to get_current_time() and get_random_number()
# to __wrap_get_current_time() and __wrap_get_random_number() instead
target_link_libraries(wrap_demo PRIVATE original_lib "-Wl,--wrap=get_current_time,--wrap=get_random_number")

# Add wrapper.c to the executable
# The wrapper functions will be linked and used instead of the original ones
target_sources(wrap_demo PRIVATE wrapper.c)

# Compile definitions for demonstration
target_compile_definitions(wrap_demo PRIVATE DEMO_MODE)

message(STATUS "Wrap demo configured:")
message(STATUS "  - Original functions: original_lib.c")
message(STATUS "  - Wrapper functions: wrapper.c")
message(STATUS "  - Main program: main.c")
message(STATUS "  - Linker option: --wrap=get_current_time,--wrap=get_random_number")

