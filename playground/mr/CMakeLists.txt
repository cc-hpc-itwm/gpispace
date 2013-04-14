# mirko.rahn@itwm.fraunhofer.de

include (SDPAFindBoost)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

find_package (UTIL REQUIRED QUIET)
include_directories (${UTIL_INCLUDE_DIR})


include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${CMAKE_SOURCE_DIR}/pnet/src)
include_directories (${UTIL_INCLUDE_DIR})

add_library (we-value
  we/type/value/show.cpp
  we/type/value/get.cpp
  we/type/value/poke.cpp
  we/type/value/read.cpp

  we/type/value/path/split.cpp
)

fhg_add_test (value.cpp
  PROJECT test_playground_mr
  LINK_LIBRARIES we-dev we-value fhg-util
  DESCRIPTION "test recursive polymorphic value"
)
