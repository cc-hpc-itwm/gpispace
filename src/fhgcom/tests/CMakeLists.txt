fhg_add_test (test_message.cpp
  PROJECT fhgcom
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhgcom
                 fhglog
)

fhg_add_test (test_p2p.cpp
  PROJECT fhgcom
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhgcom
                 fhglog
)

fhg_add_test (test_peer.cpp
  PROJECT fhgcom
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhgcom
                 fhg-util
                 fhglog
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
)
