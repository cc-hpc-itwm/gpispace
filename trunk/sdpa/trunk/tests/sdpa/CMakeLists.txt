# -*- mode: cmake; -*-
include (fhg_plugin_macros)

set(TESTS_EXAMPLE_STRESSTEST_MODULES_PATH "${CMAKE_BINARY_DIR}/application/example/stresstest")
message(STATUS "example modules are in: ${TESTS_EXAMPLE_STRESSTEST_MODULES_PATH}")

get_target_property(TESTS_FVM_PC_MODULE fvm-pc.shared LOCATION)
message(STATUS "fvm module is in: ${TESTS_FVM_PC_MODULE}")

get_target_property(TESTS_FVM_PC_FAKE_MODULE fvm-pc-fake.shared LOCATION)
message(STATUS "fvm fake module is in: ${TESTS_FVM_PC_FAKE_MODULE}")

fhg_add_plugin (fvm-fake fvm-fake.cpp
  EXPORTS FVM
  LINK_LIBRARIES ${TESTS_FVM_PC_FAKE_MODULE}
  DEPENDS fvm-pc-fake.shared
  )

get_target_property(TESTS_FVM_FAKE_PLUGIN_PATH fvm-fake-plugin LOCATION)

set(TESTS_WORKFLOWS_PATH "${CMAKE_CURRENT_BINARY_DIR}/workflows")
message(STATUS "workflows are in: ${TESTS_WORKFLOWS_PATH}")

set(TESTS_NRE_PCD_BIN_PATH "/todo/remove/dummy/nre/pcd/path")

get_target_property(TESTS_KVS_PLUGIN_PATH kvs-plugin LOCATION)
get_target_property(TESTS_GUI_PLUGIN_PATH gui-plugin LOCATION)
get_target_property(TESTS_DRTS_PLUGIN_PATH drts-plugin LOCATION)
get_target_property(TESTS_WFE_PLUGIN_PATH wfe-plugin LOCATION)
get_target_property(TESTS_WFE_TEST_PLUGIN_PATH wfe_test-plugin LOCATION)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests_config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/tests_config.hpp)

add_subdirectory(workflows)

include(pnet_macros)

pnet_compile(sdpa-stresstest ${CMAKE_CURRENT_SOURCE_DIR}/workflows/stresstest.xml
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/workflows/stresstest.pnet
  FLAGS -I ${CMAKE_SOURCE_DIR}/application/example/stresstest -I ${CMAKE_SOURCE_DIR}/application/lib 
)

set (test_srcs
  SchedulerTestImpl.cpp
  test_Config.cpp
)

ADD_LIBRARY(sdpa-test ${test_srcs})
target_link_libraries(sdpa-test
	 sdpa-daemon
	 sdpa-client
	 sdpa-events
	 sdpa-util
	 sdpa
	 ${SEDA_LIBRARY}
	 ${Boost_UNIT_TEST_LIBRARIES}
	 ${Boost_LIBRARIES})

set (tc_srcs
  test_CancelJob.cpp
  test_Config.cpp
  test_Worker.cpp
  test_JobId.cpp
  test_Scheduler.cpp
  test_LoadBalancer.cpp
  test_SerializeSharedPtr.cpp
  test_SerializeJobPtr.cpp
  test_SerializeAgents.cpp
  test_AgentsAndDrts.cpp
  test_StopRestartOrchPollingCli.cpp
  test_StopRestartOrchSubscriberCli.cpp
  test_StopRestartAgentsSubscriberCli.cpp
  test_StopRestartAgentsPollingCli.cpp
  test_ConcurrentClients.cpp
  test_MultipleMasters.cpp
  test_Topologies.cpp
  test_Subscriber.cpp
#  test_MM.cpp
)

foreach (tc_src ${tc_srcs})
  # get the filename without extension and insert the project name
  string(REGEX REPLACE "(.*/)?test_(.*)\\.c.*" "test_${PROJECT_NAME}_\\2" tc_name ${tc_src})
  message(STATUS "adding test case ${tc_name} (using ${tc_src})")

  add_executable(${tc_name} ${tc_src})
  target_link_libraries(${tc_name}
                        sdpa-daemon
                        sdpa-com
                        sdpa-client
                        sdpa-events
                        sdpa-util
                        sdpa
                        fhg-plugin
                        ${SEDA_LIBRARY}
                        ${FhgLog_LIBRARY}
                        ${WE_LIBRARIES}
                        ${Boost_UNIT_TEST_LIBRARIES}
                        ${Boost_LIBRARIES}
                        dl
                        )
  get_target_property(LOC ${tc_name} LOCATION_)
  add_test(${tc_name} ${LOC})
  add_dependencies(${tc_name} fvm-fake-plugin)
endforeach(tc_src ${tc_srcs})
