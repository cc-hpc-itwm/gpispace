# mirko.rahn@itwm.fraunhofer.de

include_directories (${UTIL_INCLUDE_DIR})

fhg_add_test (locked_parent_child_relation.cpp
  BOOST_UNIT_TEST
  PROJECT playground_mr
  LINK_LIBRARIES fhg-util
                 ${Boost_SYSTEM_LIBRARY}
)
