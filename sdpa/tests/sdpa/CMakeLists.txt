include (fhg_plugin_macros)

configure_file(in.txt ${CMAKE_BINARY_DIR}/sdpa/tests/sdpa/in.txt COPYONLY)

include_directories (${FVM_INCLUDE_DIR})
include_directories (SYSTEM ${HWLOC_INCLUDE_DIR})

set(DIR_SRC_TEST_WORKFLOWS "${CMAKE_SOURCE_DIR}/sdpa/tests/sdpa/workflows")

set(TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH "${CMAKE_CURRENT_BINARY_DIR}/capabilities")
set(TESTS_EXAMPLE_CAPABILITIES_NO_MANDATORY_MODULES_PATH "${CMAKE_CURRENT_BINARY_DIR}/capabilities_no_mandatory")
set(TESTS_EXAMPLE_INOTIFY_MODULES_PATH "${CMAKE_CURRENT_BINARY_DIR}/inotify")
set(TESTS_TRANSFORM_FILE_MODULES_PATH "${CMAKE_CURRENT_BINARY_DIR}/transform_file")
set(TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH "${CMAKE_CURRENT_BINARY_DIR}/coallocation_test")

fhg_add_plugin (fvm-fake fvm-fake.cpp LINK_LIBRARIES fvm-pc-fake.shared)

get_target_property (TESTS_FVM_FAKE_PLUGIN_PATH fvm-fake-plugin LOCATION)

include_directories (${CMAKE_CURRENT_BINARY_DIR})
include_directories (${CMAKE_CURRENT_SOURCE_DIR})

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/tests_config.hpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/tests_config.hpp
)

include (pnet_macros)

get_target_property (PNETPUT_LOCATION pnetput LOCATION)

###########################################################################################

pnet_compile (capabilities-test-wf
  ${CMAKE_SOURCE_DIR}/application/example/capabilities/capabilities.xpnet
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/capabilities-test-wf.in.pnet
  GENERATE capabilities
  INCLUDES ${CMAKE_SOURCE_DIR}/share/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/application/example
           -O3
  BUILD
  COMPONENT tests
)

add_custom_target (workflow_capabilities
  COMMAND ${PNETPUT_LOCATION}
          --if ${CMAKE_CURRENT_BINARY_DIR}/capabilities-test-wf.in.pnet
          -p n_A=20L
          -p time_A=5000L
          -p n_B=10L
          -p time_B=5000L
          --of "${CMAKE_CURRENT_BINARY_DIR}/capabilities.pnet"
  COMMENT "putting tokens into capabilities workflow"
  DEPENDS pnet-capabilities-test-wf
          pnetput
)

###########################################################################################

pnet_compile (capabilities_no_mandatory-test-wf
  ${CMAKE_SOURCE_DIR}/application/example/capabilities_no_mandatory/capabilities_no_mandatory.xml
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/capabilities_no_mandatory-test-wf.in.pnet
  GENERATE capabilities_no_mandatory
  INCLUDES ${CMAKE_SOURCE_DIR}/share/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/application/example
           -O3
  BUILD
  COMPONENT tests
)

add_custom_target (workflow_capabilities_no_mandatory
  COMMAND ${PNETPUT_LOCATION}
          --if ${CMAKE_CURRENT_BINARY_DIR}/capabilities_no_mandatory-test-wf.in.pnet
          -p n_A=10L
          -p time_A=5000L
          -p n_B=10L
          -p time_B=5000L
          --of "${CMAKE_CURRENT_BINARY_DIR}/capabilities_no_mandatory.pnet"
  COMMENT "putting tokens into capabilities_no_mandatory workflow"
  DEPENDS pnet-capabilities_no_mandatory-test-wf
          pnetput
)

################################################################################################

pnet_compile (inotify-test-wf
  ${CMAKE_SOURCE_DIR}/application/example/inotify/inotify.xml
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/inotify-test-wf.in.pnet
  GENERATE inotify
  INCLUDES ${CMAKE_SOURCE_DIR}/share/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/application/example
           -O3
  BUILD
  COMPONENT tests
)

add_custom_target (workflow_inotify
  COMMAND ${PNETPUT_LOCATION}
          --if ${CMAKE_CURRENT_BINARY_DIR}/inotify-test-wf.in.pnet
          -p N=5L
          -p watched_file=\\"inotify_test.txt\\"
          --of "${CMAKE_CURRENT_BINARY_DIR}/inotify.pnet"
  COMMENT "putting tokens into inotify workflow"
  DEPENDS pnet-inotify-test-wf
          pnetput
)

################################################################################################

pnet_compile (transform_file-test-wf
  ${CMAKE_SOURCE_DIR}/application/transform_file/xml/transform_file.xpnet
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/transform_file-test-wf.in.pnet
  GENERATE transform_file
  INCLUDES ${CMAKE_SOURCE_DIR}/share/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/fvm-pc
           -I${CMAKE_SOURCE_DIR}/application/transform_file
           -O3
  BUILD
  COMPONENT tests
)

add_custom_target (workflow_transform_file
  COMMAND ${PNETPUT_LOCATION}
          --if ${CMAKE_CURRENT_BINARY_DIR}/transform_file-test-wf.in.pnet
          -p input_file=\\"in.txt\\"
          -p output_file=\\"\\"
          -p input_n_read_slots="3L"
          -p input_n_write_slots="3L"
          -p chunk_size="512L"
          --of "${CMAKE_CURRENT_BINARY_DIR}/transform_file.pnet"
  COMMENT "putting tokens into transform_file workflow"
  DEPENDS pnet-transform_file-test-wf
          pnetput
)

################################################################################################

pnet_compile (coallocation_test-wf
  ${CMAKE_SOURCE_DIR}/application/example/coallocation_test/coallocation_test.xpnet
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/coallocation_test-wf.in.pnet
  GENERATE coallocation_test
  INCLUDES ${CMAKE_SOURCE_DIR}/share/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/fvm-pc
           -I${CMAKE_SOURCE_DIR}/application/example/coallocation_test
           -O3
  BUILD
  COMPONENT tests
)

add_custom_target (workflow_coallocation_test
  COMMAND ${PNETPUT_LOCATION}
          --if ${CMAKE_CURRENT_BINARY_DIR}/coallocation_test-wf.in.pnet
          -p n_tasks_A="20L"
          -p n_workers_A="2UL"
          -p n_tasks_B="10L"
          -p n_workers_B="3UL"
          --of "${CMAKE_CURRENT_BINARY_DIR}/coallocation_test.pnet"
  COMMENT "putting tokens into coallocation_test workflow"
  DEPENDS pnet-coallocation_test-wf
          pnetput
)

add_custom_target (workflow_coallocation_bad_test
  COMMAND ${PNETPUT_LOCATION}
          --if ${CMAKE_CURRENT_BINARY_DIR}/coallocation_test-wf.in.pnet
          -p n_tasks_A="1L"
          -p n_workers_A="0UL"
          -p n_tasks_B="1L"
          -p n_workers_B="0UL"
          --of "${CMAKE_CURRENT_BINARY_DIR}/coallocation_bad_test.pnet"
  COMMENT "putting tokens into coallocation_bad_test workflow"
  DEPENDS pnet-coallocation_test-wf
          pnetput
)

add_custom_target (workflow_coallocation_test2
  COMMAND ${PNETPUT_LOCATION}
          --if ${CMAKE_CURRENT_BINARY_DIR}/coallocation_test-wf.in.pnet
          -p n_tasks_A="1L"
          -p n_workers_A="2UL"
          -p n_tasks_B="1L"
          -p n_workers_B="3UL"
          --of "${CMAKE_CURRENT_BINARY_DIR}/coallocation_test2.pnet"
  COMMENT "putting tokens into coallocation_test2 workflow"
  DEPENDS pnet-coallocation_test-wf
          pnetput
)

################################################################################################

#test_Capabilities
fhg_add_test (test_Capabilities.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_capabilities
          workflow_capabilities_no_mandatory
)

#test_Coallocation
fhg_add_test (test_Coallocation.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_coallocation_test
)

#test_Inotify
fhg_add_test (test_Inotify.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fake-drts-kernel-plugin
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS fvm-fake-plugin
          workflow_inotify
)

#test_CancelJob
fhg_add_test (test_CancelJob.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_coallocation_test2
          workflow_transform_file
          workflow_capabilities
)

#test_CoallocationScheduler
fhg_add_test (test_CoallocationScheduler.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 ${FhgLog_LIBRARY}
)

#test_SimpleScheduler
fhg_add_test (test_SimpleScheduler.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 ${FhgLog_LIBRARY}
)

#test_Topologies
fhg_add_test (test_Topologies.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_transform_file
)

#test_Subscriber
fhg_add_test (test_Subscriber.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_transform_file
)

#test_SubmitJobFails
fhg_add_test (test_SubmitJobFails.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

#test_TransformFile
fhg_add_test (test_TransformFile.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_transform_file
  )

#test_DiscoverJobStates
fhg_add_test (test_DiscoverJobStates.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_coallocation_test
          workflow_coallocation_test2
)

#test_FailOnBadRequirements.cpp
fhg_add_test (test_FailOnBadRequirements.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_coallocation_bad_test
)

#test_Requirements.cpp
fhg_add_test (test_Requirements.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_capabilities
)
#test_ScheduleData.cpp
fhg_add_test (test_ScheduleData.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_coallocation_bad_test
          workflow_coallocation_test2
)

#test_WorkflowEngineCancel.cpp
fhg_add_test (test_WorkflowEngineCancel.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_coallocation_test
)

#test_WorkflowEngineDiscover.cpp
fhg_add_test (test_WorkflowEngineDiscover.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_coallocation_test
)