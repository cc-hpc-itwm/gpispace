add_library (test-utilities
  make.cpp
  scoped_nodefile_from_environment.cpp
  shared_directory.cpp
  source_directory.cpp
  virtual_memory_socket_name_for_localhost.cpp
)
target_include_directories (test-utilities PRIVATE "${CMAKE_SOURCE_DIR}")
target_link_libraries (test-utilities
  fhg-util
  Boost::program_options
  Boost::filesystem
)
