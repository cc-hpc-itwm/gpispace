
add_executable (interleave interleave.c)

add_executable (test_process_file_input test_process_file_input.cpp)
target_link_libraries(test_process_file_input process)
target_link_libraries(test_process_file_input ${FhgLog_LIBRARY})
target_link_libraries(test_process_file_input ${Boost_LIBRARIES})
target_link_libraries(test_process_file_input pthread)
add_dependencies (test_process_file_input interleave)

add_test ("test_process_file_input"
           test_process_file_input ${CMAKE_CURRENT_BINARY_DIR}/interleave
         )
