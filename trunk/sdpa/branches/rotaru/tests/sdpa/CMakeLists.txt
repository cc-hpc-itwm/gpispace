# -*- mode: cmake; -*-
## add the CPPUNIT stuff
include_directories(${CPPUNIT_INCLUDE_DIRS})
link_directories(${CPPUNIT_LIBRARY_DIRS})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (USE_SMC_SC)
	add_state_machine(PERFORMANCETEST_FSM_CPP PERFORMANCETEST_FSM_H PerformanceTest)
endif (USE_SMC_SC)

file(GLOB test_srcs "test_*.cpp" SchedulerTestImpl.cpp)
file(GLOB test_hdrs "*.hpp")

set(test_hdrs ${PERFORMANCETEST_FSM_H} ${test_hdrs})
set(test_srcs ${PERFORMANCETEST_FSM_CPP} ${test_srcs})

ADD_LIBRARY(sdpa-test ${test_hdrs} ${test_srcs})
target_link_libraries(sdpa-test
	 sdpa-daemon
	 sdpa-client
	 sdpa-nre
#	 sdpa-aggregator
#	 sdpa-orchestrator
	 sdpa-events
	 sdpa-util
	 sdpa
	 seda
	 seda-comm
     ${CPPUNIT_STATIC_LIBRARIES}
	 ${Boost_LIBRARIES})

add_executable(sdpa_test_suite ${CMAKE_CURRENT_SOURCE_DIR}/sdpa_test_suite.cpp)
target_link_libraries(sdpa_test_suite sdpa-test)
target_link_libraries(sdpa_test_suite dl)
target_link_libraries(sdpa_test_suite uuid)

add_executable(mini_test_suite ${CMAKE_CURRENT_SOURCE_DIR}/mini_test_suite.cpp)
target_link_libraries(mini_test_suite sdpa-test)
target_link_libraries(mini_test_suite dl)
target_link_libraries(mini_test_suite uuid)

# test module
add_library(example-mod SHARED example-mod.cpp)

# adding the tests
add_test(sdpa_test_suite ${CMAKE_CURRENT_BINARY_DIR}/sdpa_test_suite)
add_test(mini_test_suite ${CMAKE_CURRENT_BINARY_DIR}/mini_test_suite)

# copy workflows over to the build directory
add_subdirectory(workflows)
