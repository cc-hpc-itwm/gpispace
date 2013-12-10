# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (queue.cpp
  PROJECT we_mgmt_bits
  LINK_LIBRARIES ${Boost_THREAD_LIBRARY}
)

fhg_add_test (signal.cpp
  PROJECT we_mgmt_bits
)
