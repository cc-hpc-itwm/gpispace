# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (priostore.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES pnet
  PROJECT we_container
  DESCRIPTION "functionality of priority_store (transition id <-> prio)"
)
