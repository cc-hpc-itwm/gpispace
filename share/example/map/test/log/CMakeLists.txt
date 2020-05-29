extended_add_library (NAME log_map-impl
  TYPE MODULE
  SOURCES "implementation.cpp"
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}/share/example"
  LIBRARIES we-dev
)

math (EXPR size_input 50)
math (EXPR size_output 50)
math (EXPR size_block 10)
math (EXPR num_block 10)

fhg_add_test (NAME share_example_map_log
  SOURCES test.cpp
  USE_BOOST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}/share/example"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/../../"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
       --implementation "$<TARGET_FILE:log_map-impl>"
       --size-input ${size_input}
       --size-output ${size_output}
       --size-block ${size_block}
       --num-block ${num_block}
       --user-data "y()"
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  DEPENDS log_map-impl
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)
