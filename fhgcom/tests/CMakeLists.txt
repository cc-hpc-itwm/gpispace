set (tests
  test_peer.cpp
  test_message.cpp
  test_connection.cpp

  test_p2p.cpp
)

foreach (test ${tests})
  fhg_add_test (${test}
    PROJECT fhgcom
    LINK_LIBRARIES fhgcom
                   ${FhgLog_LIBRARY}
                   ${Boost_LIBRARIES}
                   ${Boost_UNIT_TEST_LIBRARIES}
  )
endforeach()
