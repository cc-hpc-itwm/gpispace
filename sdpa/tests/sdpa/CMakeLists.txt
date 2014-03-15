include_directories (${CMAKE_CURRENT_SOURCE_DIR})

fhg_add_test (test_Capabilities.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (test_Coallocation.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (test_CancelJob.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (test_CoallocationScheduler.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 ${FhgLog_LIBRARY}
)

fhg_add_test (test_Topologies.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (test_Subscriber.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (test_SubmitJobFails.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (test_DiscoverJobStates.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (test_Requirements.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (test_RestartWorkersCoalloc.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (test_RestartWorkersDummy.cpp
  PROJECT sdpa
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)
