
add_executable(simple_tcp_example
    simple_tcp_example.cc
)

target_link_libraries(simple_tcp_example
    tcp_network_lib
    Threads::Threads
    ${PLATFORM_LIBS}
)

target_include_directories(simple_tcp_example
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_definitions(simple_tcp_example
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)
