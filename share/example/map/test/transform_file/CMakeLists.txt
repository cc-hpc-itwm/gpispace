# mirko.rahn@itwm.fraunhofer.de

add_library (transform_file_type
  type.cpp
)
target_include_directories (transform_file_type
  PRIVATE "${CMAKE_SOURCE_DIR}/share/example"
)
target_link_libraries (transform_file_type
  we-dev
  Boost::filesystem
)

add_library (transform_file_map-impl SHARED
  implementation.cpp
)
target_link_libraries (transform_file_map-impl
  transform_file_type
)
target_include_directories (transform_file_map-impl
  PRIVATE "${CMAKE_SOURCE_DIR}/share/example"
)

math (EXPR size_communication_buffer "8 * 4 << 20")

math (EXPR size_input "20 << 20")
math (EXPR size_output "20 << 20")
math (EXPR size_block "2 << 20")
math (EXPR num_block "120")
math (EXPR size_virtual_memory
           "${size_input} + ${size_output} + ${size_communication_buffer}"
     )

fhg_add_test (test.cpp
  PROJECT share_example_map_transform_file
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
                      PRIVATE "${CMAKE_SOURCE_DIR}/share/example"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/../../"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-per-node ${size_virtual_memory}
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
