include (fhg_plugin_macros)

include_directories (${FVM_INCLUDE_DIR})
include_directories (SYSTEM ${HWLOC_INCLUDE_DIR})

set(TEST_WORKFLOWS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/sdpa/tests/sdpa/workflows")

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
          -p n_B=10L
          --of "${CMAKE_CURRENT_BINARY_DIR}/capabilities.pnet"
  COMMENT "putting tokens into capabilities workflow"
  DEPENDS pnet-capabilities-test-wf
          pnetput
)

###########################################################################################

fhg_add_test (test_Capabilities.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
  DEPENDS workflow_capabilities
)

fhg_add_test (test_Coallocation.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
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

fhg_add_test (test_DiscoverJobStates.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
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
)

fhg_add_test (test_RestartWorkersDummy.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)
