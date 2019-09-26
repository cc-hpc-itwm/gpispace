fhg_add_test (NAME fhgcom_message
  SOURCES message.cpp
  USE_BOOST
  LIBRARIES fhgcom
            fhglog
)

fhg_add_test (NAME fhgcom_p2p
  SOURCES p2p.cpp
  USE_BOOST
  LIBRARIES fhgcom
            fhglog
)

fhg_add_test (NAME fhgcom_peer
  SOURCES peer.cpp
  USE_BOOST
  LIBRARIES fhgcom
            fhg-util
            fhglog
            test-utilities
            Boost::filesystem
            Boost::serialization
            Boost::date_time
            Boost::thread
)
