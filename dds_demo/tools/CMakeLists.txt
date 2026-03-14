# List types tool
add_executable(list_types list_types.cpp)
target_link_libraries(list_types dds_wrapper idl_types fastdds fastcdr)

# Installation
install(TARGETS list_types DESTINATION bin/tools)
install(PROGRAMS create_idl.py validate_idl.sh DESTINATION bin/tools)
