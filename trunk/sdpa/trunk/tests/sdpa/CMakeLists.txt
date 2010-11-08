# -*- mode: cmake; -*-
## add the CPPUNIT stuff

set(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH "${CMAKE_BINARY_DIR}/application/example/stresstest")
message(STATUS "example modules are in: ${TESTS_EXAMPLE_STRESSTEST_MODULES_PATH}")

set(TESTS_FVM_PC_MODULE "${CMAKE_BINARY_DIR}/fvm-pc/fvm-pc/libfvm-pc.so")
message(STATUS "fvm module is in: ${TESTS_FVM_PC_MODULE}")

set(TESTS_FVM_PC_FAKE_MODULE "${CMAKE_BINARY_DIR}/fvm-pc/fvm-pc/fake/libfvm-pc.so")
message(STATUS "fvm fake module is in: ${TESTS_FVM_PC_FAKE_MODULE}")

set(TESTS_WORKFLOWS_PATH "${CMAKE_CURRENT_BINARY_DIR}/workflows")
message(STATUS "workflows are in: ${TESTS_WORKFLOWS_PATH}")

set(TESTS_NRE_PCD_BIN "${CMAKE_BINARY_DIR}/sdpa/sdpa/daemon/nre/nre-worker/nre-worker/nre-pcd")
message(STATUS "nre-pcd binary path set to: ${TESTS_NRE_PCD_BIN}")

include_directories(${CPPUNIT_INCLUDE_DIRS})
link_directories(${CPPUNIT_LIBRARY_DIRS})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests_config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/tests_config.hpp)

add_subdirectory(workflows)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/workflows/stresstest.pnet
	COMMAND ${CMAKE_BINARY_DIR}/xml/xml/parse/pnetc 
	ARGS -I ${CMAKE_SOURCE_DIR}/application/example/stresstest -I ${CMAKE_SOURCE_DIR}/application/lib  ${CMAKE_SOURCE_DIR}/sdpa/tests/sdpa/workflows/stresstest.xml -o ${CMAKE_CURRENT_BINARY_DIR}/workflows/stresstest.pnet
    COMMENT "Invoke command: ${CMAKE_BINARY_DIR}/xml/xml/parse/pnetc -I ${CMAKE_SOURCE_DIR}/application/example/stresstest -I ${CMAKE_SOURCE_DIR}/application/lib  ${CMAKE_SOURCE_DIR}/sdpa/tests/sdpa/workflows/stresstest.xml -o ${CMAKE_CURRENT_BINARY_DIR}/workflows/stresstest.pnet" VERBATIM
)
       
set (test_srcs
  SchedulerTestImpl.cpp
  test_Components.cpp
  test_Config.cpp
  test_JobFSM_SMC.cpp
  test_JobId.cpp
  test_LoadBalancer.cpp
  test_Scheduler.cpp
  test_SerializeDaemonComponents.cpp
  test_SerializeJobPtr.cpp
  test_SerializeSharedPtr.cpp
  test_UUID.cpp
  test_Worker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../sdpa/daemon/nre/nre-worker/nre-worker/ActivityExecutor.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/workflows/stresstest.pnet
)
                   
ADD_LIBRARY(sdpa-test ${test_srcs})
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

# adding the tests
add_test(sdpa_test_suite ${CMAKE_CURRENT_BINARY_DIR}/sdpa_test_suite)
add_test(mini_test_suite ${CMAKE_CURRENT_BINARY_DIR}/mini_test_suite)

#add_dependencies (sdpa-test ${CMAKE_CURRENT_BINARY_DIR}/workflows/stresstest.pnet)
                   