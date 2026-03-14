# Basic pub-sub example
add_executable(basic_pubsub basic_pubsub.cpp)
target_include_directories(basic_pubsub PRIVATE ${CMAKE_BINARY_DIR}/idl)
target_link_libraries(basic_pubsub dds_wrapper idl_types fastdds fastcdr)

# Multiple topics example
add_executable(multiple_topics multiple_topics.cpp)
target_include_directories(multiple_topics PRIVATE ${CMAKE_BINARY_DIR}/idl)
target_link_libraries(multiple_topics dds_wrapper idl_types fastdds fastcdr)

# Reliable communication example
add_executable(reliable_communication reliable_communication.cpp)
target_include_directories(reliable_communication PRIVATE ${CMAKE_BINARY_DIR}/idl)
target_link_libraries(reliable_communication dds_wrapper idl_types fastdds fastcdr)

# Custom QoS example
add_executable(custom_qos custom_qos.cpp)
target_include_directories(custom_qos PRIVATE ${CMAKE_BINARY_DIR}/idl)
target_link_libraries(custom_qos dds_wrapper idl_types fastdds fastcdr)

# Add new type tutorial
add_executable(add_new_type_tutorial add_new_type_tutorial.cpp)
target_include_directories(add_new_type_tutorial PRIVATE ${CMAKE_BINARY_DIR}/idl)
target_link_libraries(add_new_type_tutorial dds_wrapper idl_types fastdds fastcdr)

# Service demo (ROS-style request/response over DDS)
add_executable(service_demo service_demo.cpp)
target_include_directories(service_demo PRIVATE ${CMAKE_BINARY_DIR}/idl)
target_link_libraries(service_demo dds_wrapper idl_types fastdds fastcdr)

# Installation
install(TARGETS
    basic_pubsub
    multiple_topics
    reliable_communication
    custom_qos
    add_new_type_tutorial
    service_demo
    DESTINATION bin/examples
)
