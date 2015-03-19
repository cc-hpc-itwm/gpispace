fhg_add_test (message.cpp
  PROJECT fhgcom_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhgcom
                 fhglog
)

fhg_add_test (p2p.cpp
  PROJECT fhgcom_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhgcom
                 fhglog
)

fhg_add_test (peer.cpp
  PROJECT fhgcom_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhgcom
                 fhg-util
                 fhglog
                 Boost::filesystem
                 Boost::serialization
                 Boost::date_time
                 Boost::thread
)
