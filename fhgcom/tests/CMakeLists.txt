fhg_add_test (test_connection.cpp
  PROJECT fhgcom
  LINK_LIBRARIES fhgcom
                 ${FhgLog_LIBRARY}
                 ${Boost_LIBRARIES}
                 ${Boost_UNIT_TEST_LIBRARIES}
)

fhg_add_test (test_message.cpp
  PROJECT fhgcom
  LINK_LIBRARIES fhgcom
                 ${FhgLog_LIBRARY}
                 ${Boost_LIBRARIES}
                 ${Boost_UNIT_TEST_LIBRARIES}
)

fhg_add_test (test_p2p.cpp
  PROJECT fhgcom
  LINK_LIBRARIES fhgcom
                 ${FhgLog_LIBRARY}
                 ${Boost_LIBRARIES}
                 ${Boost_UNIT_TEST_LIBRARIES}
)

fhg_add_test (test_peer.cpp
  PROJECT fhgcom
  LINK_LIBRARIES fhgcom
                 ${FhgLog_LIBRARY}
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_LIBRARIES}
                 ${Boost_UNIT_TEST_LIBRARIES}
)
