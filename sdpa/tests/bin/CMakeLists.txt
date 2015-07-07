fhg_add_test (codec.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa fhglog
)

fhg_add_test (network_strategy.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhgcom fhglog Boost::filesystem
                 Network
)

fhg_add_test (job_states.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhg-util
)
