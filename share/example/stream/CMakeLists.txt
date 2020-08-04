math (EXPR size_slot "2 << 10")
math (EXPR num_slots "20")

fhg_add_test (NAME share_example_stream_two_module_calls
  SOURCES stream.two_module_calls.cpp
  USE_BOOST
  ARGS --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
       --num-slots ${num_slots}
       --size-slot ${size_slot}
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
  RUN_SERIAL # actually a performance test but a fundamental one we
             # don't want to ignore
)

fhg_add_test (NAME share_example_stream_one_module_call
  SOURCES stream.one_module_call.cpp
  USE_BOOST
  ARGS --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
       --num-slots ${num_slots}
       --size-slot ${size_slot}
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
  RUN_SERIAL # actually a performance test but a fundamental one we
             # don't want to ignore
)
