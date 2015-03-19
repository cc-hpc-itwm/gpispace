# mirko.rahn@itwm.fraunhofer.de

math (EXPR size_communication_buffer "8 * 4 << 20")

math (EXPR size_slot "2 << 10")
math (EXPR num_slots "20")
math (EXPR size_virtual_memory
           "(${size_slot} + 1) * ${num_slots} + ${size_communication_buffer}"
     )

fhg_add_test (stream.two_module_calls.cpp
  PROJECT share_example_stream_two_module_calls
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-per-node ${size_virtual_memory}
       --virtual-memory-startup-timeout 60
       --num-slots ${num_slots}
       --size-slot ${size_slot}
  LINK_LIBRARIES gspc
                 test-utilities
                 fhg-util
                 Boost::program_options
                 Boost::program_options
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)

fhg_add_test (stream.one_module_call.cpp
  PROJECT share_example_stream_one_module_call
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-per-node ${size_virtual_memory}
       --virtual-memory-startup-timeout 60
       --num-slots ${num_slots}
       --size-slot ${size_slot}
  LINK_LIBRARIES gspc
                 test-utilities
                 fhg-util
                 Boost::program_options
                 Boost::program_options
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)
