gspc_add_test (NAME scheduler_network_strategy
  SOURCES network_strategy.cpp
  USE_BOOST
  LIBRARIES gspc_scheduler gspc_we
            fhgcom
            gspc_testing
            PRIVATE GPISpace::SchedulerTestNetworkStrategy
)

gspc_add_test (NAME scheduler_job_states
  SOURCES job_states.cpp
  USE_BOOST
  LIBRARIES gspc_util
            gspc_testing
)
