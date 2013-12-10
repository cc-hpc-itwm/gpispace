fhg_add_test (test_codec.cpp BOOST_UNIT_TEST LINK_LIBRARIES sdpa ${FhgLog_LIBRARY})

fhg_add_test (test_network_strategy.cpp
  LINK_LIBRARIES sdpa-com sdpa
                 ${SEDA_LIBRARY} ${FhgCom_LIBRARY} ${FhgLog_LIBRARY} ${Boost_FILESYSTEM_LIBRARY}
)

fhg_add_test (test_job_states.cpp)
