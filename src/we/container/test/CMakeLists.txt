# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (priostore.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES pnet
  PROJECT test_we_container
  DESCRIPTION "functionality of priority_store (transition id <-> prio)"
)

fhg_add_test (adjacency.cpp
  PROJECT test_we_container
)
