# mirko.rahn@itwm.fraunhofer.de

extended_add_library (NAME transform_file_type
  SOURCES "type.cpp"
  INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/share/example"
  LIBRARIES we-dev
            Boost::filesystem
)

extended_add_library (NAME transform_file_map-impl
  TYPE MODULE
  SOURCES "implementation.cpp"
  LIBRARIES transform_file_type
  INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/share/example"
)

math (EXPR size_input "20 << 20")
math (EXPR size_output "20 << 20")
math (EXPR size_block "2 << 20")
math (EXPR num_block "120")

fhg_add_test (test.cpp
  PROJECT share_example_map_transform_file
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}/share/example"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/../../"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
       --implementation "$<TARGET_FILE:transform_file_map-impl>"
       --size-input ${size_input}
       --size-output ${size_output}
       --size-block ${size_block}
       --num-block ${num_block}
  LINK_LIBRARIES gspc
                 test-utilities
                 transform_file_type
                 fhg-util
                 Boost::program_options
                 Boost::filesystem
  DEPENDS transform_file_map-impl
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)
