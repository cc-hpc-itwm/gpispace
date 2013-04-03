# mirko.rahn@itwm.fraunhofer.de

include (SDPAFindBoost)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${CMAKE_SOURCE_DIR}/pnet/src)

add_library (we-value
  we/type/value.cpp
  we/type/value/show.cpp
  we/type/value/get.cpp
  we/type/value/put.cpp
  we/type/value/poke.cpp
)

fhg_add_test (value.cpp
  PROJECT test_playground_mr
  LINK_LIBRARIES we-dev we-value
  DESCRIPTION "test recursive polymorphic value"
)
