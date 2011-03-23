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
#	COMMAND ${CMAKE_BINARY_DIR}/xml/xml/parse/pnetc
	COMMAND pnetc
	ARGS -I ${CMAKE_SOURCE_DIR}/application/example/stresstest -I ${CMAKE_SOURCE_DIR}/application/lib  ${CMAKE_SOURCE_DIR}/sdpa/tests/sdpa/workflows/stresstest.xml -o ${CMAKE_CURRENT_BINARY_DIR}/workflows/stresstest.pnet
    COMMENT "Invoke command: pnetc -I ${CMAKE_SOURCE_DIR}/application/example/stresstest -I ${CMAKE_SOURCE_DIR}/application/lib  ${CMAKE_SOURCE_DIR}/sdpa/tests/sdpa/workflows/stresstest.xml -o ${CMAKE_CURRENT_BINARY_DIR}/workflows/stresstest.pnet" VERBATIM
)
else (USE_REAL_WE )
	configure_file(${CMAKE_SOURCE_DIR}/sdpa/tests/sdpa/workflows/stresstest.xml ${CMAKE_CURRENT_BINARY_DIR}/workflows/stresstest.pnet)
endif (USE_REAL_WE)

set (test_srcs
  SchedulerTestImpl.cpp
  test_Config.cpp
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
#     ${FVM_FAKE_STATIC_LIBRARY}
#     ${MMGR_LIBRARY}
	 ${Boost_LIBRARIES})


add_executable(test_config_suite test_Config.cpp)
target_link_libraries(test_config_suite  seda sdpa-daemon
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 #uuid
									 ${Boost_LIBRARIES}
                                                                         ${WE_PRECOMPILE_LIBRARIES})
add_test( test_config_suite test_config_suite )

add_executable(test_worker_suite test_Worker.cpp)
target_link_libraries(test_worker_suite  seda sdpa-daemon
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 #uuid
									 ${Boost_LIBRARIES}
                                                                         ${WE_PRECOMPILE_LIBRARIES})
add_test( test_worker_suite test_worker_suite )

add_executable(test_jobid_suite test_JobId.cpp)
target_link_libraries(test_jobid_suite  seda sdpa-daemon
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 #uuid
									 ${Boost_LIBRARIES}
                                                                         ${WE_PRECOMPILE_LIBRARIES})
add_test( test_jobid_suite test_jobid_suite )

add_executable(test_scheduler_suite test_Scheduler.cpp)
target_link_libraries(test_scheduler_suite  seda sdpa-daemon
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 #uuid
									 ${Boost_LIBRARIES}
                                                                         ${WE_PRECOMPILE_LIBRARIES})
add_test( test_scheduler_suite test_scheduler_suite )

add_executable(test_lb_suite test_LoadBalancer.cpp)
target_link_libraries(test_lb_suite  seda sdpa-daemon
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 #uuid
									 ${Boost_LIBRARIES}
                                                                         ${WE_PRECOMPILE_LIBRARIES})
add_test( test_lb_suite test_lb_suite )

add_executable(test_serialize_sharedptr_suite test_SerializeSharedPtr.cpp)
target_link_libraries(test_serialize_sharedptr_suite  seda sdpa-daemon
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 #uuid
									 ${Boost_LIBRARIES}
                                                                         ${WE_PRECOMPILE_LIBRARIES})
add_test( test_serialize_sharedptr_suite test_serialize_sharedptr_suite )


add_executable(test_serialize_jobptr_suite test_SerializeJobPtr.cpp)
target_link_libraries(test_serialize_jobptr_suite  seda sdpa-daemon
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 #uuid
									 ${Boost_LIBRARIES}
                                                                         ${WE_PRECOMPILE_LIBRARIES})
add_test(test_serialize_jobptr_suite test_serialize_jobptr_suite)

set( test_serialize_agents_srcs
  	test_SerializeAgents.cpp
)

add_executable(test_serialize_agents_suite ${test_serialize_agents_srcs})
target_link_libraries(test_serialize_agents_suite  seda sdpa-daemon
									 sdpa-client
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 #uuid
									 ${Boost_LIBRARIES}
                                                                         ${WE_PRECOMPILE_LIBRARIES})
add_test(test_serialize_agents_suite test_serialize_agents_suite)

set( test_agents_and_act_exec_srcs
  	test_AgentsAndActExec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sdpa/daemon/nre/nre-worker/nre-worker/ActivityExecutor.cpp
)

if (USE_REAL_WE)
	add_executable(test_agents_and_act_exec_suite ${test_agents_and_act_exec_srcs})
	target_link_libraries(test_agents_and_act_exec_suite  seda sdpa-daemon
										 sdpa-client
										 sdpa-events
										 sdpa-util
										 sdpa
										 dl
										 #uuid
										 ${Boost_LIBRARIES}
                                                                                 ${WE_PRECOMPILE_LIBRARIES})
	add_test(test_agents_and_act_exec_suite test_agents_and_act_exec_suite)
endif (USE_REAL_WE)

set( test_agents_src
  	test_Agents.cpp
)

add_executable(test_agents_suite ${test_agents_src})
target_link_libraries(test_agents_suite  seda sdpa-daemon
									 sdpa-client
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 #uuid
									 ${Boost_LIBRARIES}
                                                                         ${WE_PRECOMPILE_LIBRARIES})
add_test( test_agents_suite test_agents_suite )

#test_StopRestartAgents
set( test_stop_restart_agents_srcs test_StopRestartAgents.cpp )

add_executable(test_stop_restart_agents ${test_stop_restart_agents_srcs})
target_link_libraries(test_stop_restart_agents  seda sdpa-daemon
									 sdpa-client
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 #uuid
									 ${Boost_LIBRARIES}
                                                                         ${WE_PRECOMPILE_LIBRARIES})
add_test( test_stop_restart_agents test_stop_restart_agents )


#test_BackupRecover
set( test_backup_recover_srcs test_BackupRecover.cpp )

add_executable(test_backup_recover ${test_backup_recover_srcs})
target_link_libraries(test_backup_recover  seda sdpa-daemon
									 sdpa-client
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 #uuid
									 ${Boost_LIBRARIES}
                                                                         ${WE_PRECOMPILE_LIBRARIES})
add_test( test_backup_recover test_backup_recover )

#test_JobSubmToWeFails
#set( test_JobSubmToWeFails_srcs test_JobSubmToWeFails.cpp )
#add_executable(test_JobSubmToWeFails ${test_JobSubmToWeFails_srcs})
#target_link_libraries(test_JobSubmToWeFails  seda sdpa-daemon
#									 sdpa-client
#									 sdpa-events
#									 sdpa-util
#									 sdpa
#									 dl
#									 ${Boost_LIBRARIES}
#                                                                        ${WE_PRECOMPILE_LIBRARIES})
#add_test( test_JobSubmToWeFails test_JobSubmToWeFails )

#test_ConcurrentClients
set( test_ConcurrentClients_srcs test_ConcurrentClients.cpp )
add_executable(test_concurrent_clients ${test_ConcurrentClients_srcs})
target_link_libraries(test_concurrent_clients  seda sdpa-daemon
									 sdpa-client
									 sdpa-events
									 sdpa-util
									 sdpa
									 dl
									 #uuid
									 ${Boost_LIBRARIES}
${WE_PRECOMPILE_LIBRARIES})
add_test( test_concurrent_clients test_concurrent_clients )

#test_backend
#set( test_Backend_srcs test_Backend.cpp )
#add_executable(test_backend ${test_Backend_srcs})
#target_link_libraries(test_backend  ${FhgLog_LIBRARY} ${Boost_LIBRARIES})
#add_test( test_backend test_backend )
