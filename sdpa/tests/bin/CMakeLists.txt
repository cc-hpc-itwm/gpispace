fhg_add_test (test_codec.cpp BOOST_UNIT_TEST LINK_LIBRARIES sdpa fhglog we-dev)

fhg_add_test (test_network_strategy.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa we-dev
                 fhgcom fhglog ${Boost_FILESYSTEM_LIBRARY}
)

fhg_add_test (test_job_states.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhg-util
)
