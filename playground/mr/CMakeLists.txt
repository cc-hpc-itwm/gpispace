# mirko.rahn@itwm.fraunhofer.de

include (SDPAFindBoost)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

fhg_add_test (value.cpp
  PROJECT test_playground_mr
  DESCRIPTION "test recursive polymorphic value"
)
