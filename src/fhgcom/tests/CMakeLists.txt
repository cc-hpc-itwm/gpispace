fhg_add_test (NAME fhgcom_p2p
  SOURCES p2p.cpp
  USE_BOOST
  LIBRARIES fhgcom
            Util::Generic-Headers
)

fhg_add_test (NAME fhgcom_peer
  SOURCES peer.cpp
  USE_BOOST
  LIBRARIES fhgcom
            fhg-util
            test-util-certificates
            Boost::filesystem
            Boost::serialization
            Boost::date_time
            Boost::thread
            OpenSSL::SSL
            OpenSSL::Crypto
)
