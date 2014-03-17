include_directories (${CMAKE_CURRENT_SOURCE_DIR})

fhg_add_test (Capabilities.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (Coallocation.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (CancelJob.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (CoallocationScheduler.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 ${FhgLog_LIBRARY}
)

fhg_add_test (Topologies.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (Subscriber.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (SubmitJobFails.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (DiscoverJobStates.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (Requirements.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (RestartWorkersCoalloc.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)

fhg_add_test (RestartWorkersDummy.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
                 fhg-plugin
                 ${FhgLog_LIBRARY}
)
