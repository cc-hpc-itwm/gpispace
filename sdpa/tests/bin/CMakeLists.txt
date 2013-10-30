fhg_add_test (test_codec.cpp LINK_LIBRARIES sdpa-events sdpa ${FhgLog_LIBRARY})

fhg_add_test (test_network_strategy.cpp
  LINK_LIBRARIES sdpa-com sdpa-events sdpa
                 ${SEDA_LIBRARY} ${FhgCom_LIBRARY} ${FhgLog_LIBRARY}
)

fhg_add_test (test_job_states.cpp)
