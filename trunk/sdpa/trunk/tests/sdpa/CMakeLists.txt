# -*- mode: cmake; -*-
## add the CPPUNIT stuff
include_directories(${CPPUNIT_INCLUDE_DIRS})
link_directories(${CPPUNIT_LIBRARY_DIRS})
link_libraries(${CPPUNIT_LIBRARIES})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_state_machine(PERFORMANCETEST_FSM_CPP PERFORMANCETEST_FSM_H PerformanceTest)

file(GLOB test_srcs "test_*.cpp" SchedulerTestImpl.cpp)
file(GLOB test_hdrs "*.hpp")

set(test_hdrs ${PERFORMANCETEST_FSM_H} ${test_hdrs})
set(test_srcs ${PERFORMANCETEST_FSM_CPP} ${test_srcs})

ADD_LIBRARY(sdpa-test ${test_hdrs} ${test_srcs})
add_executable(sdpa_test_suite ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_dependencies(sdpa_test_suite generate_PerformanceTest)
target_link_libraries(sdpa_test_suite sdpa-test)
target_link_libraries(sdpa_test_suite sdpa)
target_link_libraries(sdpa_test_suite sdpa-modules)
target_link_libraries(sdpa_test_suite sdpa-daemon)
target_link_libraries(sdpa_test_suite sdpa-events)
target_link_libraries(sdpa_test_suite sdpa-jobFSM_SMC)
target_link_libraries(sdpa_test_suite sdpa-daemonFSM_SMC)
target_link_libraries(sdpa_test_suite sdpa-util)
target_link_libraries(sdpa_test_suite dl)
target_link_libraries(sdpa_test_suite uuid)

# test module
add_library(example-mod SHARED example-mod.cpp)

# adding the tests
add_test(sdpa_test_suite ${CMAKE_CURRENT_BINARY_DIR}/sdpa_test_suite)

# copy workflows over to the build directory
add_subdirectory(workflows)
