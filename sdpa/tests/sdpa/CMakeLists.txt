include (fhg_plugin_macros)

configure_file(in.txt ${CMAKE_BINARY_DIR}/sdpa/tests/sdpa/in.txt COPYONLY)

include_directories (${FVM_INCLUDE_DIR})
include_directories (SYSTEM ${HWLOC_INCLUDE_DIR})

set(TEST_WORKFLOWS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/sdpa/tests/sdpa/workflows")

set(TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH "${CMAKE_CURRENT_BINARY_DIR}/capabilities")
set(TESTS_EXAMPLE_CAPABILITIES_NO_MANDATORY_MODULES_PATH "${CMAKE_CURRENT_BINARY_DIR}/capabilities_no_mandatory")
set(TESTS_EXAMPLE_INOTIFY_MODULES_PATH "${CMAKE_CURRENT_BINARY_DIR}/inotify")
set(TESTS_TRANSFORM_FILE_MODULES_PATH "${CMAKE_CURRENT_BINARY_DIR}/transform_file")
set(TESTS_EXAMPLE_coallocation_MODULES_PATH "${CMAKE_CURRENT_BINARY_DIR}/coallocation")

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
  ${TEST_WORKFLOWS_SOURCE_DIR}/capabilities.xpnet
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/capabilities-test-wf.in.pnet
  GENERATE capabilities
  INCLUDES ${CMAKE_SOURCE_DIR}/share/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -O3
  BUILD
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
  ${TEST_WORKFLOWS_SOURCE_DIR}/capabilities_no_mandatory.xpnet
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/capabilities_no_mandatory-test-wf.in.pnet
  GENERATE capabilities_no_mandatory
  INCLUDES ${CMAKE_SOURCE_DIR}/share/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -O3
  BUILD
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
  ${TEST_WORKFLOWS_SOURCE_DIR}/inotify.xpnet
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/inotify-test-wf.in.pnet
  GENERATE inotify
  INCLUDES ${CMAKE_SOURCE_DIR}/share/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -O3
  BUILD
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
  ${TEST_WORKFLOWS_SOURCE_DIR}/transform_file/transform_file.xpnet
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/transform_file-test-wf.in.pnet
  GENERATE transform_file
  INCLUDES ${CMAKE_SOURCE_DIR}/share/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/fvm-pc
           -I${TEST_WORKFLOWS_SOURCE_DIR}/transform_file
           -O3
  BUILD
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

pnet_compile (coallocation-wf
  ${TEST_WORKFLOWS_SOURCE_DIR}/coallocation.xpnet
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/coallocation-wf.in.pnet
  GENERATE coallocation
  INCLUDES ${CMAKE_SOURCE_DIR}/share/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/fvm-pc
           -O3
  BUILD
)

add_custom_target (workflow_coallocation
  COMMAND ${PNETPUT_LOCATION}
          --if ${CMAKE_CURRENT_BINARY_DIR}/coallocation-wf.in.pnet
          -p n_tasks="1L"
          -p n_workers="2UL"
          --of "${CMAKE_CURRENT_BINARY_DIR}/coallocation.pnet"
  COMMENT "putting tokens into coallocation workflow"
  DEPENDS pnet-coallocation-wf
          pnetput
)

################################################################################################

fhg_add_test (test_Capabilities.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_capabilities
          workflow_capabilities_no_mandatory
)

fhg_add_test (test_Coallocation.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_coallocation
)

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

fhg_add_test (test_CancelJob.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (test_CoallocationScheduler.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 ${FhgLog_LIBRARY}
)

fhg_add_test (test_Topologies.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (test_Subscriber.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (test_SubmitJobFails.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (test_TransformFile.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
  				 fake-drts-kernel-plugin
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS fvm-fake-plugin
          workflow_transform_file
  )

fhg_add_test (test_DiscoverJobStates.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_coallocation
)

fhg_add_test (test_Requirements.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_capabilities
)

fhg_add_test (test_RestartWorkersCoalloc.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_coallocation
)

fhg_add_test (test_RestartWorkersDummy.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)
