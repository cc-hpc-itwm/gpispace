# mirko.rahn@itwm.fraunhofer.de

include_directories (${CMAKE_SOURCE_DIR}/share/example)
include_directories (${FhgLog_INCLUDE_DIR})

add_library (log_map-impl SHARED
  implementation.cpp
)
set_target_properties (log_map-impl PROPERTIES OUTPUT_NAME map-impl)

include_directories (${UTIL_INCLUDE_DIR})
include_directories (${CMAKE_SOURCE_DIR})

get_target_property (location_log_map-impl log_map-impl LOCATION)
get_target_property (location_virtual_memory_manager gpi-space LOCATION)

math (EXPR size_communication_buffer "8 * 4 << 20")

math (EXPR size_input 50)
math (EXPR size_output 50)
math (EXPR size_block 10)
math (EXPR num_block 10)
math (EXPR size_virtual_memory
           "${size_input} + ${size_output} + ${size_communication_buffer}"
     )

fhg_add_test (test.cpp
  PROJECT share_example_map_log
  BOOST_UNIT_TEST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/../../"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-manager "${location_virtual_memory_manager}"
       --virtual-memory-per-node ${size_virtual_memory}
       --implementation "${location_log_map-impl}"
       --size-input ${size_input}
       --size-output ${size_output}
       --size-block ${size_block}
       --num-block ${num_block}
       --user-data "y()"
  LINK_LIBRARIES gspc
                 test-utilities
                 ${UTIL_LIBRARIES}
                 ${Boost_PROGRAM_OPTIONS_LIBRARIES}
                 ${Boost_FILESYSTEM_LIBRARIES}
  RESOURCE_LOCK starts_drts_environment
)
