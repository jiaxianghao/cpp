cmake_minimum_required(VERSION 3.16)
project(malloc_replace LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(global_new_delete_demo
    src/global_new_delete_demo.cpp
    src/global_new_delete.cpp
)
target_compile_options(global_new_delete_demo PRIVATE -Wall -Wextra)

add_custom_target(run_global
    COMMAND $<TARGET_FILE:global_new_delete_demo>
    DEPENDS global_new_delete_demo
    USES_TERMINAL
    VERBATIM
)

add_executable(container_allocator_demo src/container_allocator_demo.cpp)
target_compile_options(container_allocator_demo PRIVATE -Wall -Wextra)

add_custom_target(run_container
    COMMAND $<TARGET_FILE:container_allocator_demo>
    DEPENDS container_allocator_demo
    USES_TERMINAL
    VERBATIM
)

add_executable(demo src/demo.c)
target_compile_options(demo PRIVATE -Wall -Wextra -fno-builtin -UNDEBUG)
target_link_libraries(demo PRIVATE ${CMAKE_DL_LIBS})

add_library(demo_allocator SHARED src/allocator.c)
target_compile_options(demo_allocator PRIVATE -Wall -Wextra -fno-builtin)

add_custom_target(run
    COMMAND $<TARGET_FILE:demo>
    COMMAND ${CMAKE_COMMAND} -E env
        "LD_PRELOAD=$<TARGET_FILE:demo_allocator>" $<TARGET_FILE:demo>
    DEPENDS demo demo_allocator
    USES_TERMINAL
    VERBATIM
)
