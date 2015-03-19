fhg_add_test (codec.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa fhglog we-dev
)

fhg_add_test (network_strategy.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa we-dev
                 fhgcom fhglog Boost::filesystem
                 Network
)

fhg_add_test (job_states.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhg-util
)
