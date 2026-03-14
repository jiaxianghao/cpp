set(WRAPPER_SOURCES
    DDSManager.cpp
    ConfigManager.cpp
    Logger.cpp
)

# Create static library
add_library(dds_wrapper STATIC ${WRAPPER_SOURCES})

target_link_libraries(dds_wrapper
    fastdds
    fastcdr
    idl_types
)

target_include_directories(dds_wrapper PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
