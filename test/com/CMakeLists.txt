gspc_add_test (NAME gspc_test_com_p2p
  SOURCES p2p.cpp
  USE_BOOST
  LIBRARIES gspc_com
            gspc_util
            gspc_testing
)

gspc_add_test (NAME gspc_test_com_peer
  SOURCES peer.cpp
  USE_BOOST
  LIBRARIES gspc_com
            gspc_util
            gspc_testing
            Boost::serialization
            Boost::date_time
            Boost::thread
            OpenSSL::SSL
            OpenSSL::Crypto
)
