# -*- mode: cmake; -*-

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

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests_config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/tests_config.hpp)

add_subdirectory(workflows)

if (USE_REAL_WE)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/workflows/stresstest.pnet
	DEPENDS ${CMAKE_SOURCE_DIR}/sdpa/tests/sdpa/workflows/stresstest.xml
	DEPENDS pnetc
	COMMAND ${CMAKE_BINARY_DIR}/xml/xml/parse/pnetc
#	COMMAND pnetc
	ARGS -I ${CMAKE_SOURCE_DIR}/application/example/stresstest -I ${CMAKE_SOURCE_DIR}/application/lib  ${CMAKE_SOURCE_DIR}/sdpa/tests/sdpa/workflows/stresstest.xml -o ${CMAKE_CURRENT_BINARY_DIR}/workflows/stresstest.pnet
    COMMENT "Invoke command: ${CMAKE_BINARY_DIR}/xml/xml/parse/pnetc -I ${CMAKE_SOURCE_DIR}/application/example/stresstest -I ${CMAKE_SOURCE_DIR}/application/lib  ${CMAKE_SOURCE_DIR}/sdpa/tests/sdpa/workflows/stresstest.xml -o ${CMAKE_CURRENT_BINARY_DIR}/workflows/stresstest.pnet" VERBATIM
)
else (USE_REAL_WE )
	configure_file(${CMAKE_SOURCE_DIR}/sdpa/tests/sdpa/workflows/stresstest.xml ${CMAKE_CURRENT_BINARY_DIR}/workflows/stresstest.pnet)
endif (USE_REAL_WE)

set (test_srcs
  SchedulerTestImpl.cpp
  test_Config.cpp
)

ADD_LIBRARY(sdpa-test ${test_srcs} ${CMAKE_CURRENT_BINARY_DIR}/workflows/stresstest.pnet)
target_link_libraries(sdpa-test
	 sdpa-daemon
	 sdpa-client
	 sdpa-events
	 sdpa-util
	 sdpa
	 seda
	 ${Boost_UNIT_TEST_LIBRARIES}
	 ${Boost_LIBRARIES})

set (tc_srcs
  test_Config.cpp
  test_Worker.cpp
  test_JobId.cpp
  test_Scheduler.cpp
  test_LoadBalancer.cpp
  test_SerializeSharedPtr.cpp
  test_SerializeJobPtr.cpp
  test_SerializeAgents.cpp
  test_Agents.cpp
  test_StopRestartAgents.cpp
  test_ConcurrentClients.cpp
  test_MultipleMasters.cpp
  test_PushCap
)

foreach (tc_src ${tc_srcs})
  # get the filename without extension and insert the project name
  string(REGEX REPLACE "(.*/)?test_(.*)\\.c.*" "test_${PROJECT_NAME}_\\2" tc_name ${tc_src})
  message(STATUS "adding test case ${tc_name} (using ${tc_src})")

  add_executable(${tc_name} ${tc_src})
  target_link_libraries(${tc_name}
                        seda
                        sdpa-daemon
                        sdpa-client
                        sdpa-events
                        sdpa-util
                        sdpa
                        ${FhgLog_LIBRARY}
                        ${Boost_UNIT_TEST_LIBRARIES}
                        ${Boost_LIBRARIES}
                        ${WE_PRECOMPILE_LIBRARIES}
                        dl
                        )
  get_target_property(LOC ${tc_name} LOCATION_)
  add_test(${tc_name} ${LOC})
endforeach(tc_src ${tc_srcs})

if (USE_REAL_WE)
  set( test_agents_and_act_exec_srcs
    test_AgentsAndActExec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sdpa/daemon/nre/nre-worker/nre-worker/ActivityExecutor.cpp
)

add_executable(test_agents_and_act_exec_suite ${test_agents_and_act_exec_srcs} )
target_link_libraries(  test_agents_and_act_exec_suite
                        seda
                        sdpa-daemon
                        sdpa-client
                        sdpa-events
                        sdpa-util
                        sdpa
                        dl
                        ${Boost_UNIT_TEST_LIBRARIES}
                        ${Boost_LIBRARIES}
                        ${WE_PRECOMPILE_LIBRARIES})
  add_test(test_agents_and_act_exec_suite test_agents_and_act_exec_suite)
endif (USE_REAL_WE)
