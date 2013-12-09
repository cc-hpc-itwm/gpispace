fhg_add_test (test_codec.cpp LINK_LIBRARIES sdpa ${FhgLog_LIBRARY} ${Boost_UNIT_TEST_LIBRARIES} ${Boost_LIBRARIES})

fhg_add_test (test_network_strategy.cpp
  LINK_LIBRARIES sdpa-com sdpa
                 ${SEDA_LIBRARY} ${FhgCom_LIBRARY} ${FhgLog_LIBRARY} ${Boost_FILESYSTEM_LIBRARY}
)

fhg_add_test (test_job_states.cpp)
