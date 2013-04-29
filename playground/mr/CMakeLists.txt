# mirko.rahn@itwm.fraunhofer.de

include (SDPAFindBoost)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

find_package (UTIL REQUIRED QUIET)
include_directories (${UTIL_INCLUDE_DIR})

include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${CMAKE_SOURCE_DIR}/pnet/src)
include_directories (${UTIL_INCLUDE_DIR})

add_library (we
  we/exception.cpp
  we/require_type.cpp
  we/type/signature/dump.cpp
  we/type/signature/show.cpp
  we/type/value/name.cpp
  we/type/value/name_of.cpp
  we/type/value/of_type.cpp
  we/type/value/path/join.cpp
  we/type/value/path/split.cpp
  we/type/value/peek.cpp
  we/type/value/poke.cpp
  we/type/value/read.cpp
  we/type/value/show.cpp
)

fhg_add_test (we/type/test/value.cpp
  PROJECT test_we_type
  LINK_LIBRARIES we we-dev fhg-util
)

fhg_add_test (we/type/test/signature.cpp
  PROJECT test_we_type
  LINK_LIBRARIES we we-dev fhg-util
)

fhg_add_test (we/test/require_type.cpp
  PROJECT test_we
  LINK_LIBRARIES we we-dev fhg-util
)
