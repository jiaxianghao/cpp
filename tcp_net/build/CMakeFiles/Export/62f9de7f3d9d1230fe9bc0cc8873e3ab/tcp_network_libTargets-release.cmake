#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "tcp_network_lib::tcp_network_lib" for configuration "Release"
set_property(TARGET tcp_network_lib::tcp_network_lib APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(tcp_network_lib::tcp_network_lib PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libtcp_network_lib.a"
  )

list(APPEND _cmake_import_check_targets tcp_network_lib::tcp_network_lib )
list(APPEND _cmake_import_check_files_for_tcp_network_lib::tcp_network_lib "${_IMPORT_PREFIX}/lib/libtcp_network_lib.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
