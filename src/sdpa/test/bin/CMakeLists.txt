fhg_add_test (NAME sdpa_network_strategy
  SOURCES network_strategy.cpp
  USE_BOOST
  LIBRARIES sdpa we-dev
            fhgcom
            test-utilities
            Boost::filesystem
            PRIVATE GPISpace::SDPATestNetworkStrategy
)

fhg_add_test (NAME sdpa_job_states
  SOURCES job_states.cpp
  USE_BOOST
  LIBRARIES fhg-util
)
