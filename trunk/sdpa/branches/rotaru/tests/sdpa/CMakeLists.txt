# -*- mode: cmake; -*-

file(GLOB test_srcs "test_*.cpp")
file(GLOB test_hdrs "*.hpp")
set(test_hdrs "${CMAKE_CURRENT_SOURCE_DIR}/PerformanceTest_sm.h" ${test_hdrs}) 
set(test_srcs "${CMAKE_CURRENT_SOURCE_DIR}/PerformanceTest_sm.cpp" ${test_srcs})
add_state_machine(PerformanceTest)
#SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/PerformanceTest_sm.h PROPERTIES GENERATED 1)
#SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/PerformanceTest_sm.cpp PROPERTIES GENERATED 1)

ADD_LIBRARY(sdpa-test ${test_hdrs} ${test_srcs})
#add_executable(sdpa_test_suite ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${test_hdrs} ${test_srcs})
add_executable(sdpa_test_suite ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_dependencies(sdpa_test_suite generate_PerformanceTest)
target_link_libraries(sdpa_test_suite sdpa-test)
target_link_libraries(sdpa_test_suite sdpa)
target_link_libraries(sdpa_test_suite sdpa-modules)
target_link_libraries(sdpa_test_suite sdpa-wf)
target_link_libraries(sdpa_test_suite sdpa-daemon)

target_link_libraries(sdpa_test_suite sdpa-events)
target_link_libraries(sdpa_test_suite sdpa-orchFSM_BSC)
target_link_libraries(sdpa_test_suite sdpa-orchFSM_SMC)
target_link_libraries(sdpa_test_suite sdpa-jobFSM_BSC)
target_link_libraries(sdpa_test_suite sdpa-jobFSM_SMC)

target_link_libraries(sdpa_test_suite dl)

# test module
add_library(example-mod SHARED ExampleModule.cpp)

add_executable(sdpa_test_tool sdpa_test_tool.cpp)
target_link_libraries(sdpa_test_tool sdpa sdpa-wf sdpa-daemon)

# adding the tests
add_test(sdpa_test_suite ${CMAKE_CURRENT_BINARY_DIR}/sdpa_test_suite)
add_test(sdpa_test_tool ${CMAKE_CURRENT_BINARY_DIR}/sdpa_test_tool)
