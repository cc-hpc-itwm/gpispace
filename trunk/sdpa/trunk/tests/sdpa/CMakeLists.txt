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
	DEPENDS ${CMAKE_SOURCE_DIR}/sdpa/tests/sdpa/workflows/stresstest.xml
	DEPENDS pnetc
	COMMAND ${CMAKE_BINARY_DIR}/xml/xml/parse/pnetc 
	ARGS -I ${CMAKE_SOURCE_DIR}/application/example/stresstest -I ${CMAKE_SOURCE_DIR}/application/lib  ${CMAKE_SOURCE_DIR}/sdpa/tests/sdpa/workflows/stresstest.xml -o ${CMAKE_CURRENT_BINARY_DIR}/workflows/stresstest.pnet
    COMMENT "Invoke command: ${CMAKE_BINARY_DIR}/xml/xml/parse/pnetc -I ${CMAKE_SOURCE_DIR}/application/example/stresstest -I ${CMAKE_SOURCE_DIR}/application/lib  ${CMAKE_SOURCE_DIR}/sdpa/tests/sdpa/workflows/stresstest.xml -o ${CMAKE_CURRENT_BINARY_DIR}/workflows/stresstest.pnet" VERBATIM
)
       
set (test_srcs
  SchedulerTestImpl.cpp
  test_Config.cpp
  test_JobFSM_SMC.cpp
  test_JobId.cpp
  test_LoadBalancer.cpp
  test_Scheduler.cpp
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
     ${CPPUNIT_STATIC_LIBRARIES}
#     ${FVM_FAKE_STATIC_LIBRARY}
#     ${MMGR_LIBRARY}
	 ${Boost_LIBRARIES})


# adding the tests
add_test(sdpa_test_suite ${CMAKE_CURRENT_BINARY_DIR}/sdpa_test_suite)


add_executable(test_jobid_suite test_JobId.cpp)
target_link_libraries(test_jobid_suite  seda sdpa-daemon
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 uuid
									 ${Boost_LIBRARIES})					 
add_test( test_jobid_suite ${CMAKE_CURRENT_BINARY_DIR}/test_jobid_suite )

add_executable(test_scheduler_suite test_Scheduler.cpp)
target_link_libraries(test_scheduler_suite  seda sdpa-daemon
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 uuid
									 ${Boost_LIBRARIES})					 
add_test( test_scheduler_suite ${CMAKE_CURRENT_BINARY_DIR}/test_scheduler_suite )

add_executable(test_lb_suite test_LoadBalancer.cpp)
target_link_libraries(test_lb_suite  seda sdpa-daemon
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 uuid
									 ${Boost_LIBRARIES})
add_test( test_lb_suite ${CMAKE_CURRENT_BINARY_DIR}/test_lb_suite )

add_executable(test_serialize_sharedptr_suite test_SerializeSharedPtr.cpp)
target_link_libraries(test_serialize_sharedptr_suite  seda sdpa-daemon
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 uuid
									 ${Boost_LIBRARIES})
add_test( test_serialize_sharedptr_suite ${CMAKE_CURRENT_BINARY_DIR}/test_serialize_sharedptr_suite )


add_executable(test_serialize_jobptr_suite test_SerializeJobPtr.cpp)
target_link_libraries(test_serialize_jobptr_suite  seda sdpa-daemon
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 uuid
									 ${Boost_LIBRARIES})		 
add_test( test_serialize_jobptr_suite ${CMAKE_CURRENT_BINARY_DIR}/test_serialize_jobptr_suite )

set( test_serialize_agents_srcs
  	test_SerializeAgents.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sdpa/daemon/nre/nre-worker/nre-worker/ActivityExecutor.cpp
)

add_executable(test_serialize_agents_suite ${test_serialize_agents_srcs})
target_link_libraries(test_serialize_agents_suite  seda sdpa-daemon
									 sdpa-client
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 uuid
									 ${Boost_LIBRARIES})	 
add_test( test_agents_suite ${CMAKE_CURRENT_BINARY_DIR}/test_serialize_agents_suite )

set( test_agents_srcs
  	test_Agents.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sdpa/daemon/nre/nre-worker/nre-worker/ActivityExecutor.cpp
)

add_executable(test_agents_suite ${test_agents_srcs})
target_link_libraries(test_agents_suite  seda sdpa-daemon
									 sdpa-client
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 uuid
									 ${Boost_LIBRARIES})		 
add_test( test_agents_suite ${CMAKE_CURRENT_BINARY_DIR}/sdpa_test_agents_suite )

set( test_orch_empty_we_srcs
  	test_OrchDifferentWEs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sdpa/daemon/nre/nre-worker/nre-worker/ActivityExecutor.cpp
)


add_executable(test_orchestrator_empty_we ${test_orch_empty_we_srcs})
target_link_libraries(test_orchestrator_empty_we  seda sdpa-daemon
									 sdpa-client
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 uuid
									 ${Boost_LIBRARIES})
add_test( test_orchestrator_empty_we ${CMAKE_CURRENT_BINARY_DIR}/test_orchestrator_empty_we )

#add_dependencies (sdpa-test ${CMAKE_CURRENT_BINARY_DIR}/workflows/stresstest.pnet)
                   