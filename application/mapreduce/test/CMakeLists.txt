
include_directories (${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../util)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable (test_merge_and_reduce_buff test_merge_and_reduce_buff.cpp)
target_link_libraries(test_merge_and_reduce_buff ${FhgLog_LIBRARY} ${Boost_LIBRARIES})
