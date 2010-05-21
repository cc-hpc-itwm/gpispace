# -*- mode: cmake; -*-
## add the CPPUNIT stuff

set(TESTS_KDM_MODULES_PATH "${CMAKE_BINARY_DIR}/we/kdm/mod")
message(STATUS "kdm modules are in: ${TESTS_KDM_MODULES_PATH}")

set(TESTS_KDM_FAKE_MODULES_PATH "${TESTS_KDM_MODULES_PATH}/fake")
message(STATUS "kdm fake modules are in: ${TESTS_KDM_FAKE_MODULES_PATH}")

set(TESTS_FVM_PC_MODULE "${CMAKE_BINARY_DIR}/fvm-pc/fvm-pc/libfvm-pc.so")
message(STATUS "fvm module is in: ${TESTS_FVM_PC_MODULE}")

set(TESTS_FVM_PC_FAKE_MODULE "${CMAKE_BINARY_DIR}/fvm-pc/fvm-pc/fake/libfvm-pc.so")
message(STATUS "fvm fake module is in: ${TESTS_FVM_PC_FAKE_MODULE}")

set(TESTS_WORKFLOWS_PATH "${CMAKE_CURRENT_BINARY_DIR}/workflows")
message(STATUS "workflows are in: ${TESTS_WORKFLOWS_PATH}")

set(TESTS_NRE_PCD_BIN_PATH "${CMAKE_BINARY_DIR}/sdpa/sdpa/daemon/nre/nre-worker/nre-worker")
message(STATUS "nre-pcd binary path set to: ${TESTS_NRE_PCD_BIN_PATH}")

include_directories(${CPPUNIT_INCLUDE_DIRS})
link_directories(${CPPUNIT_LIBRARY_DIRS})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests_config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/tests_config.hpp)

if (USE_SMC_SC)
	add_state_machine(PERFORMANCETEST_FSM_CPP PERFORMANCETEST_FSM_H PerformanceTest)
endif (USE_SMC_SC)

file(GLOB test_srcs "test_*.cpp" SchedulerTestImpl.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../sdpa/daemon/nre/nre-worker/nre-worker/ActivityExecutor.cpp)
file(GLOB test_hdrs "*.hpp")

set(test_hdrs ${PERFORMANCETEST_FSM_H} ${test_hdrs})
set(test_srcs ${PERFORMANCETEST_FSM_CPP} ${test_srcs})

ADD_LIBRARY(sdpa-test ${test_hdrs} ${test_srcs})
target_link_libraries(sdpa-test
	 sdpa-daemon
	 sdpa-client
#	 sdpa-nre
#	 sdpa-aggregator
#	 sdpa-orchestrator
	 sdpa-events
	 sdpa-util
	 sdpa
	 seda
	 seda-comm
     ${CPPUNIT_STATIC_LIBRARIES}
#     ${FVM_FAKE_STATIC_LIBRARY}
#     ${MMGR_LIBRARY}
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
#add_library(example-mod SHARED example-mod.cpp)

# adding the tests
add_test(sdpa_test_suite ${CMAKE_CURRENT_BINARY_DIR}/sdpa_test_suite)
add_test(mini_test_suite ${CMAKE_CURRENT_BINARY_DIR}/mini_test_suite)

configure_file(run_pcd.fake.test ${CMAKE_CURRENT_BINARY_DIR}/run_pcd.fake.test COPYONLY)

# copy workflows over to the build directory
add_subdirectory(workflows)
