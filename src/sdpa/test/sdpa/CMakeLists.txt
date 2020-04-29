extended_add_library (NAME SDPATestUtilities
  NAMESPACE GPISpace
  SOURCES "utils.cpp"
  LIBRARIES sdpa
)

fhg_add_test (NAME sdpa_AddNewWorkers
  SOURCES AddNewWorkers.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_CancelJob
  SOURCES CancelJob.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_Capabilities
  SOURCES Capabilities.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_Coallocation
  SOURCES Coallocation.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_CoallocationScheduler
  SOURCES CoallocationScheduler.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_daemon_message_flow
  SOURCES daemon_message_flow.cpp
  DESCRIPTION "Message flow between client/orchestrator/agent and children/parents"
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_DiscoverJobStates
  SOURCES DiscoverJobStates.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_InvalidNbWorkersReq
  SOURCES InvalidNbWorkersReq.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_RestartWorkersCoalloc
  SOURCES RestartWorkersCoalloc.cpp
  NAME restart_workers_with_coallocation_workflow
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_RestartWorkersDummy
  SOURCES RestartWorkersDummy.cpp
  NAME restart_worker_with_dummy_workflow
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_Scheduler_no_preferences.performance
  SOURCES Scheduler_no_preferences.performance.cpp
  USE_BOOST
  PERFORMANCE_TEST
  RUN_SERIAL
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_Scheduler_with_preferences.performance
  SOURCES Scheduler_with_preferences.performance.cpp
  USE_BOOST
  PERFORMANCE_TEST
  RUN_SERIAL
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_Subscriber
  SOURCES Subscriber.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_Topologies
  SOURCES Topologies.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_WorkerManager
  SOURCES WorkerManager.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_WorkerReportsBacklogFull
  SOURCES WorkerReportsBacklogFull.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME remove_workers_in_a_coallocation_job_and_add_them_again
  SOURCES remove_workers_in_a_coallocation_job.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME forbid_equal_preferences_for_tasks
  SOURCES forbid_equal_preferences_for_tasks.cpp
  USE_BOOST
  LIBRARIES sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_Preferences
  SOURCES Preferences.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_network_basics
  SOURCES network_basics.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)
