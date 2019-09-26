fhg_add_test (NAME sdpa_network_strategy
  SOURCES network_strategy.cpp
  USE_BOOST
  LIBRARIES sdpa we-dev
            fhgcom fhglog 
            test-utilities
            Boost::filesystem
)

fhg_add_test (NAME sdpa_job_states
  SOURCES job_states.cpp
  USE_BOOST
  LIBRARIES fhg-util
)