fhg_add_test (CancelJob.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LINK_LIBRARIES sdpa
                 fhglog
                 Network
)

fhg_add_test (Capabilities.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LINK_LIBRARIES sdpa
                 fhglog
                 Network
)

fhg_add_test (Coallocation.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LINK_LIBRARIES sdpa
                 fhglog
                 Network
)

fhg_add_test (CoallocationScheduler.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LINK_LIBRARIES sdpa
                 fhglog
                 Network
)

fhg_add_test (daemon_message_flow.cpp
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LINK_LIBRARIES sdpa
)

fhg_add_test (DiscoverJobStates.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LINK_LIBRARIES sdpa
                 fhglog
                 Network
)

fhg_add_test (InvalidNbWorkersReq.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LINK_LIBRARIES sdpa
                 fhglog
                 Network
)

fhg_add_test (RestartWorkersCoalloc.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LINK_LIBRARIES sdpa
                 fhglog
                 Network
)

fhg_add_test (RestartWorkersDummy.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LINK_LIBRARIES sdpa
                 fhglog
                 Network
)

fhg_add_test (SubmitJobFails.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LINK_LIBRARIES sdpa
                 fhglog
                 Network
)

fhg_add_test (Subscriber.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LINK_LIBRARIES sdpa
                 fhglog
                 Network
)

fhg_add_test (Topologies.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LINK_LIBRARIES sdpa
                 Network
                 fhglog
)

fhg_add_test (WorkerManager.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LINK_LIBRARIES sdpa
                 fhglog
                 Network
)

fhg_add_test (WorkerReportsBacklogFull.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  LINK_LIBRARIES sdpa
                 fhglog
                 Network
)
