
include_directories (${CMAKE_CURRENT_SOURCE_DIR})

add_executable (interleave interleave.c)
add_executable (deinterleave deinterleave.c)

add_executable (rotate rotate.c)
set_target_properties (rotate PROPERTIES COMPILE_FLAGS -std=c99)

set (tests
     test_process_file_input
     test_process_file_output
     test_process_all
     test_process_redefine
     test_process_big
    )

foreach (test ${tests})
  add_executable (${test} "${test}.cpp")
  target_link_libraries(${test} process)
  target_link_libraries(${test} ${FhgLog_LIBRARY})
  target_link_libraries(${test} ${Boost_LIBRARIES})
  target_link_libraries(${test} pthread)
endforeach (test ${tests})

add_dependencies (test_process_file_input interleave)
add_test ("test_process_file_input"
           test_process_file_input ${CMAKE_CURRENT_BINARY_DIR}/interleave
         )

add_dependencies (test_process_file_output deinterleave)
add_test ("test_process_file_output"
           test_process_file_output ${CMAKE_CURRENT_BINARY_DIR}/deinterleave
         )

add_dependencies (test_process_all rotate)
add_test ("test_process_all"
           test_process_all ${CMAKE_CURRENT_BINARY_DIR}/rotate
         )

add_test ("test_process_redefine" test_process_redefine)
add_test ("test_process_big" test_process_big 1048576)

fhg_add_test (test_basics.cpp
  LINK_LIBRARIES process ${FhgLog_LIBRARY} ${Boost_LIBRARIES} pthread
)
