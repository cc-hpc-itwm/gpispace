gspc_add_library (NAME SchedulerTestUtilities
  TYPE STATIC
  NAMESPACE GPISpace
  SOURCES "utils.cpp"
  LIBRARIES gspc_scheduler PUBLIC GPISpace::SchedulerTestNetworkStrategy gspc_testing
)

gspc_add_test (NAME scheduler_AddNewWorkers
  SOURCES AddNewWorkers.cpp
  USE_BOOST
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME scheduler_CancelJob
  SOURCES CancelJob.cpp
  USE_BOOST
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME scheduler_Coallocation
  SOURCES Coallocation.cpp
  USE_BOOST
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME scheduler_CoallocationScheduler
  SOURCES CoallocationScheduler.cpp
  USE_BOOST
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME scheduler_Scheduler
  SOURCES Scheduler.cpp
  USE_BOOST
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME scheduler_daemon_message_flow
  SOURCES daemon_message_flow.cpp
  DESCRIPTION "Message flow between client/agent and children/parents"
  USE_BOOST
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME scheduler_InvalidNbWorkersReq
  SOURCES InvalidNbWorkersReq.cpp
  USE_BOOST
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME scheduler_RestartWorkersCoalloc
  SOURCES RestartWorkersCoalloc.cpp
  NAME restart_workers_with_coallocation_workflow
  USE_BOOST
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME scheduler_RestartWorkersDummy
  SOURCES RestartWorkersDummy.cpp
  NAME restart_worker_with_dummy_workflow
  USE_BOOST
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME scheduler_Scheduler_single_allocation_no_preferences.performance
  SOURCES Scheduler_single_allocation_no_preferences.performance.cpp
  USE_BOOST
  PERFORMANCE_TEST
  RUN_SERIAL
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME scheduler_Scheduler_single_allocation_with_preferences.performance
  SOURCES Scheduler_single_allocation_with_preferences.performance.cpp
  USE_BOOST
  PERFORMANCE_TEST
  RUN_SERIAL
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME scheduler_Scheduler_coallocation_no_preferences.performance
  SOURCES Scheduler_coallocation_no_preferences.performance.cpp
  USE_BOOST
  PERFORMANCE_TEST
  RUN_SERIAL
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME scheduler_Scheduler_greedy_no_preferences.performance
  SOURCES Scheduler_greedy_no_preferences.performance.cpp
  USE_BOOST
  PERFORMANCE_TEST
  RUN_SERIAL
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME scheduler_Subscriber
  SOURCES Subscriber.cpp
  USE_BOOST
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME scheduler_WorkerManager
  SOURCES WorkerManager.cpp
  USE_BOOST
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME remove_workers_in_a_coallocation_job_and_add_them_again
  SOURCES remove_workers_in_a_coallocation_job.cpp
  USE_BOOST
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME forbid_equal_preferences_for_tasks
  SOURCES forbid_equal_preferences_for_tasks.cpp
  USE_BOOST
  LIBRARIES gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME scheduler_Preferences
  SOURCES Preferences.cpp
  USE_BOOST
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)

gspc_add_test (NAME scheduler_network_basics
  SOURCES network_basics.cpp
  USE_BOOST
  LIBRARIES GPISpace::SchedulerTestUtilities
            gspc_scheduler
            gspc_testing
)
