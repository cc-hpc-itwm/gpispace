fhg_add_test (network_strategy.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa we-dev
                 fhgcom fhglog Boost::filesystem
)

fhg_add_test (job_states.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhg-util
)
