# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (queue.cpp
  PROJECT we_mgmt_bits
  LINK_LIBRARIES ${Boost_THREAD_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_test (signal.cpp
  BOOST_UNIT_TEST
  PROJECT we_mgmt_bits
)
