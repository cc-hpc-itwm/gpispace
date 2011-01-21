
include_directories (${CMAKE_CURRENT_SOURCE_DIR})

add_executable (interleave interleave.c)
add_executable (deinterleave deinterleave.c)

add_executable (rotate rotate.c)
set_target_properties (rotate PROPERTIES COMPILE_FLAGS -std=c99)

add_executable (test_process_file_input test_process_file_input.cpp)
target_link_libraries(test_process_file_input process)
target_link_libraries(test_process_file_input ${FhgLog_LIBRARY})
target_link_libraries(test_process_file_input ${Boost_LIBRARIES})
target_link_libraries(test_process_file_input pthread)
add_dependencies (test_process_file_input interleave)

add_test ("test_process_file_input"
           test_process_file_input ${CMAKE_CURRENT_BINARY_DIR}/interleave
         )

add_executable (test_process_file_output test_process_file_output.cpp)
target_link_libraries(test_process_file_output process)
target_link_libraries(test_process_file_output ${FhgLog_LIBRARY})
target_link_libraries(test_process_file_output ${Boost_LIBRARIES})
target_link_libraries(test_process_file_output pthread)
add_dependencies (test_process_file_output deinterleave)

add_test ("test_process_file_output"
           test_process_file_output ${CMAKE_CURRENT_BINARY_DIR}/deinterleave
         )

add_executable (test_process_all test_process_all.cpp)
target_link_libraries(test_process_all process)
target_link_libraries(test_process_all ${FhgLog_LIBRARY})
target_link_libraries(test_process_all ${Boost_LIBRARIES})
target_link_libraries(test_process_all pthread)
add_dependencies (test_process_all rotate)

add_test ("test_process_all"
           test_process_all ${CMAKE_CURRENT_BINARY_DIR}/rotate
         )
