fhg_add_test (NAME sdpa_AddNewWorkers
  SOURCES AddNewWorkers.cpp
  USE_BOOST
  INCLUDE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARIES sdpa
            fhglog
)

fhg_add_test (NAME sdpa_CancelJob
  SOURCES CancelJob.cpp
  USE_BOOST
  INCLUDE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARIES sdpa
            fhglog
)

fhg_add_test (NAME sdpa_Capabilities
  SOURCES Capabilities.cpp
  USE_BOOST
  INCLUDE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARIES sdpa
            fhglog
)

fhg_add_test (NAME sdpa_Coallocation
  SOURCES Coallocation.cpp
  USE_BOOST
  INCLUDE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARIES sdpa
            fhglog
)

fhg_add_test (NAME sdpa_CoallocationScheduler
  SOURCES CoallocationScheduler.cpp
  USE_BOOST
  INCLUDE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARIES sdpa
            fhglog
)

fhg_add_test (NAME sdpa_daemon_message_flow
  SOURCES daemon_message_flow.cpp
  DESCRIPTION "Message flow between client/orchestrator/agent and children/parents"
  USE_BOOST
  INCLUDE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARIES sdpa
)

fhg_add_test (NAME sdpa_DiscoverJobStates
  SOURCES DiscoverJobStates.cpp
  USE_BOOST
  INCLUDE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARIES sdpa
            fhglog
)

fhg_add_test (NAME sdpa_InvalidNbWorkersReq
  SOURCES InvalidNbWorkersReq.cpp
  USE_BOOST
  INCLUDE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARIES sdpa
            fhglog
)

fhg_add_test (NAME sdpa_RestartWorkersCoalloc
  SOURCES RestartWorkersCoalloc.cpp
  NAME restart_workers_with_coallocation_workflow
  USE_BOOST
  INCLUDE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARIES sdpa
            fhglog
)

fhg_add_test (NAME sdpa_RestartWorkersDummy
  SOURCES RestartWorkersDummy.cpp
  NAME restart_worker_with_dummy_workflow
  USE_BOOST
  INCLUDE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARIES sdpa
            fhglog
)

fhg_add_test (NAME sdpa_Subscriber
  SOURCES Subscriber.cpp
  USE_BOOST
  INCLUDE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARIES sdpa
            fhglog
)

fhg_add_test (NAME sdpa_Topologies
  SOURCES Topologies.cpp
  USE_BOOST
  INCLUDE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARIES sdpa
            fhglog
)

fhg_add_test (NAME sdpa_WorkerManager
  SOURCES WorkerManager.cpp
  USE_BOOST
  INCLUDE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARIES sdpa
            fhglog
)

fhg_add_test (NAME sdpa_WorkerReportsBacklogFull
  SOURCES WorkerReportsBacklogFull.cpp
  USE_BOOST
  INCLUDE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LIBRARIES sdpa
            fhglog
)
