extended_add_library (NAME test-utilities
  SOURCES "make.cpp"
          "scoped_nodefile_from_environment.cpp"
          "shared_directory.cpp"
          "source_directory.cpp"
          "virtual_memory_socket_name_for_localhost.cpp"
  INCLUDE_DIRECTORIES PUBLIC "${CMAKE_SOURCE_DIR}"
  LIBRARIES fhg-util
            Boost::program_options
            Boost::filesystem
)
