# -*- mode: cmake; -*-
include (fhg_plugin_macros)

add_subdirectory(workflows)

find_package(FVM REQUIRED QUIET)
include_directories(${FVM_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/application/transform_file)

set(TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH "${CMAKE_CURRENT_BINARY_DIR}/capabilities")
set(TESTS_EXAMPLE_CAPABILITIES_NO_MANDATORY_MODULES_PATH "${CMAKE_CURRENT_BINARY_DIR}/capabilities_no_mandatory")
set(TESTS_EXAMPLE_ATOMIC_MODULES_PATH "${CMAKE_CURRENT_BINARY_DIR}/atomic")
set(TESTS_N_ATOMIC_TASKS 100)
set(TESTS_EXAMPLE_INOTIFY_MODULES_PATH "${CMAKE_CURRENT_BINARY_DIR}/inotify")
set(TESTS_TRANSFORM_FILE_MODULES_PATH "${CMAKE_CURRENT_BINARY_DIR}/transform_file")
set(TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH "${CMAKE_CURRENT_BINARY_DIR}/coallocation_test")

get_target_property(TESTS_FVM_PC_MODULE fvm-pc.shared LOCATION)
get_target_property(TESTS_FVM_PC_FAKE_MODULE fvm-pc-fake.shared LOCATION)

fhg_add_plugin (fvm-fake fvm-fake.cpp
  EXPORTS FVM
  LINK_LIBRARIES ${TESTS_FVM_PC_FAKE_MODULE}
  DEPENDS fvm-pc-fake.shared
  )

get_target_property(TESTS_FVM_FAKE_PLUGIN_PATH fvm-fake-plugin LOCATION)

set(TESTS_WORKFLOWS_PATH "${CMAKE_CURRENT_BINARY_DIR}/workflows")

get_target_property(TESTS_KVS_PLUGIN_PATH kvs-plugin LOCATION)
get_target_property(TESTS_GUI_PLUGIN_PATH gui-plugin LOCATION)
get_target_property(TESTS_DRTS_PLUGIN_PATH drts-plugin LOCATION)
get_target_property(TESTS_WFE_PLUGIN_PATH wfe-plugin LOCATION)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests_config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/tests_config.hpp)
include(pnet_macros)

get_target_property(PNETPUT_LOCATION pnetput LOCATION)

###########################################################################################
pnet_compile(capabilities-test-wf ${CMAKE_SOURCE_DIR}/application/example/capabilities/capabilities.xpnet
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/workflows/capabilities-test-wf.in.pnet
  GENERATE capabilities
  INCLUDES ${CMAKE_SOURCE_DIR}/application/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/application/example
           -O3
  BUILD
  COMPONENT tests
)

get_target_property(CAPABILITY_PNET_LOCATION pnet-capabilities-test-wf LOCATION)
get_target_property(__PNET_GEN_OUTPUTS pnet-capabilities-test-wf GEN_OUTPUTS)
set(command ${PNETPUT_LOCATION} --if  "${CAPABILITY_PNET_LOCATION}" -p n_A=100L -p time_A=50000L -p n_B=50L -p time_B=50000L --of "${CMAKE_CURRENT_BINARY_DIR}/workflows/capabilities.pnet")

add_custom_command(OUTPUT ${__PNET_GEN_OUTPUTS}
                   COMMAND ${command}
                   COMMENT "putting tokens into capabilities workflow: ${command}"
                   APPEND
)
add_dependencies(pnet-capabilities-test-wf pnetput)
###########################################################################################
pnet_compile(capabilities_no_mandatory-test-wf ${CMAKE_SOURCE_DIR}/application/example/capabilities_no_mandatory/capabilities_no_mandatory.xml
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/workflows/capabilities_no_mandatory-test-wf.in.pnet
  GENERATE capabilities_no_mandatory
  INCLUDES ${CMAKE_SOURCE_DIR}/application/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/application/example
           -O3
  BUILD
  COMPONENT tests
)

get_target_property(CAPABILITY_NO_MANDATORY_PNET_LOCATION pnet-capabilities_no_mandatory-test-wf LOCATION)
get_target_property(__PNET_GEN_OUTPUTS pnet-capabilities_no_mandatory-test-wf GEN_OUTPUTS)
set(command ${PNETPUT_LOCATION} --if  "${CAPABILITY_NO_MANDATORY_PNET_LOCATION}" -p n_A=10L -p time_A=50000L -p n_B=10L -p time_B=50000L --of "${CMAKE_CURRENT_BINARY_DIR}/workflows/capabilities_no_mandatory.pnet")

add_custom_command(OUTPUT ${__PNET_GEN_OUTPUTS}
                   COMMAND ${command}
                   COMMENT "putting tokens into capabilities_no_mandatory workflow: ${command}"
                   APPEND
)
add_dependencies(pnet-capabilities_no_mandatory-test-wf pnetput)
############################################################################################
pnet_compile(atomic-test-wf ${CMAKE_SOURCE_DIR}/application/example/atomic/atomic.xpnet
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/workflows/atomic-test-wf.in.pnet
  GENERATE atomic
  INCLUDES ${CMAKE_SOURCE_DIR}/application/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/application/example
           -O3
  BUILD
  COMPONENT tests
)

get_target_property(ATOMIC_PNET_LOCATION pnet-atomic-test-wf LOCATION)
get_target_property(__PNET_GEN_OUTPUTS pnet-atomic-test-wf GEN_OUTPUTS)
set(command ${PNETPUT_LOCATION}
       --if  "${ATOMIC_PNET_LOCATION}"
       -p n_A=${TESTS_N_ATOMIC_TASKS}L
       -p n_B=${TESTS_N_ATOMIC_TASKS}L
       -p shared_file=\\"atomic_test.txt\\"
       --of "${CMAKE_CURRENT_BINARY_DIR}/workflows/atomic.pnet"
   )

add_custom_command(OUTPUT ${__PNET_GEN_OUTPUTS}
                   COMMAND ${command}
                   COMMENT "putting tokens into atomic workflow: ${command}"
                   APPEND
)

set(command ${PNETPUT_LOCATION}
       --if  "${ATOMIC_PNET_LOCATION}"
       -p n_A=10L
       -p n_B=10L
       -p shared_file=\\"atomic_wfe_plugin_test.txt\\"
       --of "${CMAKE_CURRENT_BINARY_DIR}/workflows/atomic_wfe_plugin_test.pnet"
   )

add_custom_command(OUTPUT ${__PNET_GEN_OUTPUTS}
                   COMMAND ${command}
                   COMMENT "putting tokens into atomic workflow: ${command}"
                   APPEND
)

add_dependencies(pnet-atomic-test-wf pnetput)
################################################################################################
pnet_compile(inotify-test-wf ${CMAKE_SOURCE_DIR}/application/example/inotify/inotify.xml
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/workflows/inotify-test-wf.in.pnet
  GENERATE inotify
  INCLUDES ${CMAKE_SOURCE_DIR}/application/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/application/example
           -O3
  BUILD
  COMPONENT tests
)

get_target_property(INOTIFY_PNET_LOCATION pnet-inotify-test-wf LOCATION)
get_target_property(__PNET_GEN_OUTPUTS pnet-inotify-test-wf GEN_OUTPUTS)
set(command ${PNETPUT_LOCATION} --if  "${INOTIFY_PNET_LOCATION}" -p N=5L -p watched_file=\\"inotify_test.txt\\" --of "${CMAKE_CURRENT_BINARY_DIR}/workflows/inotify.pnet")

add_custom_command(OUTPUT ${__PNET_GEN_OUTPUTS}
                   COMMAND ${command}
                   COMMENT "putting tokens into inotify workflow: ${command}"
                   APPEND
)
add_dependencies(pnet-inotify-test-wf pnetput)
################################################################################################
#transform
pnet_compile(transform_file-test-wf ${CMAKE_SOURCE_DIR}/application/transform_file/xml/transform_file.xpnet
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/workflows/transform_file-test-wf.in.pnet
  GENERATE transform_file
  INCLUDES ${CMAKE_SOURCE_DIR}/application/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/fvm-pc
           -I${CMAKE_SOURCE_DIR}/application/transform_file
           -O3
  BUILD
  COMPONENT tests
)

get_target_property(TRANSFORM_FILE_PNET_LOCATION pnet-transform_file-test-wf LOCATION)
get_target_property(__PNET_GEN_OUTPUTS pnet-transform_file-test-wf GEN_OUTPUTS)
set(command ${PNETPUT_LOCATION} --if "${TRANSFORM_FILE_PNET_LOCATION}" -p input_file=\\"in.txt\\"
                                                                        -p output_file=\\"\\"
                                                                        -p input_n_read_slots="3L"
                                                                        -p input_n_write_slots="3L"
                                                                        -p chunk_size="512L"
                                --of "${CMAKE_CURRENT_BINARY_DIR}/workflows/transform_file.pnet")

add_custom_command(OUTPUT ${__PNET_GEN_OUTPUTS}
                   COMMAND ${command}
                   COMMENT "putting tokens into transform_file workflow: ${command}"
                   APPEND
)
add_dependencies(pnet-transform_file-test-wf pnetput)
################################################################################################
#coallocation_test
pnet_compile(coallocation_test-wf ${CMAKE_SOURCE_DIR}/application/example/coallocation_test/coallocation_test.xpnet
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/workflows/coallocation_test-wf.in.pnet
  GENERATE coallocation_test
  INCLUDES ${CMAKE_SOURCE_DIR}/application/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/fvm-pc
           -I${CMAKE_SOURCE_DIR}/application/example/coallocation_test
           -O3
  BUILD
  COMPONENT tests
)

get_target_property(COALLOCATION_TEST_PNET_LOCATION pnet-coallocation_test-wf LOCATION)
get_target_property(__PNET_GEN_OUTPUTS pnet-coallocation_test-wf GEN_OUTPUTS)
set(command ${PNETPUT_LOCATION} --if "${COALLOCATION_TEST_PNET_LOCATION}" 	-p n_tasks_A="20L"
                                                                        	-p n_workers_A="2L"
                                                                        	-p n_tasks_B="10L"
                                                                        	-p n_workers_B="3L"
                                --of "${CMAKE_CURRENT_BINARY_DIR}/workflows/coallocation_test.pnet")

add_custom_command(OUTPUT ${__PNET_GEN_OUTPUTS}
                   COMMAND ${command}
                   COMMENT "putting tokens into transform_file workflow: ${command}"
                   APPEND
)
add_dependencies(pnet-coallocation_test-wf pnetput)
################################################################################################
set (tests
  test_Atomic.cpp
  test_BackupRecover.cpp
  test_Capabilities.cpp
  test_Coallocation.cpp
  test_StopRestartAgentsCoalloc.cpp
  test_Inotify.cpp
  test_CancelJob.cpp
  test_Config.cpp
  test_Worker.cpp
  test_JobId.cpp
  test_Scheduler.cpp
  test_LostAgentRegistrationAck.cpp
  test_SerializeSharedPtr.cpp
  test_SerializeJobPtr.cpp
  test_SerializeAgents.cpp
  test_AgentsAndDrts.cpp
  test_StopRestartOrchPollingCli.cpp
  test_StopRestartOrchSubscriberCli.cpp
  test_StopRestartAgentsSubscriberCli.cpp
  test_StopRestartAgentsPollingCli.cpp
  test_StopRestartDrtsSubscriberCli.cpp
  test_StopRestartDrtsPollingCli.cpp
  test_MultipleMasters.cpp
  test_Topologies.cpp
  test_Subscriber.cpp
  #test_CannonOnTorus.cpp
  test_TransformFile.cpp
  test_wfe_plugin.cpp
)

foreach (test ${tests})
    fhg_add_test (${test}
      PROJECT sdpa
      LINK_LIBRARIES sdpa-daemon
                     sdpa-com
                     sdpa-client
                     sdpa-events
                     sdpa-util
                     sdpa
                     fhg-plugin
                     fhg-revision
                     ${SEDA_LIBRARY}
                     ${FhgLog_LIBRARY}
                     we-dev
                     pnet
                     we-dev
                     fhg-util
                     ${Boost_UNIT_TEST_LIBRARIES}
                     ${Boost_LIBRARIES}
                     ${CMAKE_DL_LIBS}
      DEPENDS drts-plugin
              fvm-fake-plugin
              pnet-capabilities-test-wf
              pnet-capabilities_no_mandatory-test-wf
              pnet-atomic-test-wf
              pnet-inotify-test-wf
              pnet-transform_file-test-wf
              pnet-coallocation_test-wf
     )
endforeach()
