# mirko.rahn@itwm.fraunhofer.de

include_directories ("${CMAKE_SOURCE_DIR}")
include_directories ("${UTIL_INCLUDE_DIR}")
include_directories ("${FVM_INCLUDE_DIR}")

add_library (producer producer.cpp)

math (EXPR size_communication_buffer "8 * 4 << 20")

math (EXPR size_slot "2 << 10")
math (EXPR num_slots "20")
math (EXPR size_virtual_memory
           "(${size_slot} + 1) * ${num_slots} + ${size_communication_buffer}"
     )

fhg_add_test (stream.two_module_calls.cpp
  PROJECT share_example_stream_two_module_calls
  BOOST_UNIT_TEST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-per-node ${size_virtual_memory}
       --virtual-memory-startup-timeout 60
       --virtual-memory-port 10820
       --num-slots ${num_slots}
       --size-slot ${size_slot}
  LINK_LIBRARIES gspc
                 test-utilities
                 producer
                 ${UTIL_LIBRARIES}
                 ${Boost_PROGRAM_OPTIONS_LIBRARIES}
                 ${Boost_FILESYSTEM_LIBRARIES}
  RESOURCE_LOCK starts_drts_environment
  REQUIRES_INSTALLATION
)

fhg_add_test (stream.one_module_call.cpp
  PROJECT share_example_stream_one_module_call
  BOOST_UNIT_TEST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-per-node ${size_virtual_memory}
       --virtual-memory-startup-timeout 60
       --virtual-memory-port 10820
       --num-slots ${num_slots}
       --size-slot ${size_slot}
  LINK_LIBRARIES gspc
                 test-utilities
                 producer
                 ${UTIL_LIBRARIES}
                 ${Boost_PROGRAM_OPTIONS_LIBRARIES}
                 ${Boost_FILESYSTEM_LIBRARIES}
  RESOURCE_LOCK starts_drts_environment
  REQUIRES_INSTALLATION
)