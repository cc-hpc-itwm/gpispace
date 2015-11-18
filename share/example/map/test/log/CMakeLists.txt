# mirko.rahn@itwm.fraunhofer.de

extended_add_library (NAME log_map-impl
  TYPE MODULE
  SOURCES "implementation.cpp"
  INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/share/example"
)

math (EXPR size_input 50)
math (EXPR size_output 50)
math (EXPR size_block 10)
math (EXPR num_block 10)
math (EXPR size_virtual_memory
           "${size_input} + ${size_output}"
     )

fhg_add_test (test.cpp
  PROJECT share_example_map_log
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
                      PRIVATE "${CMAKE_SOURCE_DIR}/share/example"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/../../"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-per-node ${size_virtual_memory}
       --virtual-memory-startup-timeout 60
       --implementation "$<TARGET_FILE:log_map-impl>"
       --size-input ${size_input}
       --size-output ${size_output}
       --size-block ${size_block}
       --num-block ${num_block}
       --user-data "y()"
  LINK_LIBRARIES gspc
                 test-utilities
                 fhg-util
                 Boost::program_options
                 Boost::filesystem
  DEPENDS log_map-impl
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)
