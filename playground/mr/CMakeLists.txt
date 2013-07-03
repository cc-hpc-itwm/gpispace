# mirko.rahn@itwm.fraunhofer.de

include (SDPAFindBoost)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

find_package (UTIL REQUIRED QUIET)
include_directories (${UTIL_INCLUDE_DIR})

include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${CMAKE_SOURCE_DIR}/pnet/src)
include_directories (${UTIL_INCLUDE_DIR})

add_library (we2
  we2/exception.cpp
  we2/field.cpp
  we2/require_type.cpp
  we2/signature_of.cpp
  we2/type/signature/complete.cpp
  we2/type/signature/cpp.cpp
  we2/type/signature/dump.cpp
  we2/type/signature/is_literal.cpp
  we2/type/signature/name.cpp
  we2/type/signature/show.cpp
  we2/type/signature/signature.cpp
  we2/type/value/name.cpp
  we2/type/value/name_of.cpp
  we2/type/value/of_type.cpp
  we2/type/value/path/append.cpp
  we2/type/value/path/join.cpp
  we2/type/value/path/split.cpp
  we2/type/value/peek.cpp
  we2/type/value/poke.cpp
  we2/type/value/read.cpp
  we2/type/value/show.cpp
)

fhg_add_test (we2/type/test/value.cpp
  PROJECT test_we_type
  LINK_LIBRARIES we2 we-dev fhg-util
)

fhg_add_test (we2/type/test/signature.cpp
  PROJECT test_we_type
  LINK_LIBRARIES we2 we-dev fhg-util
)

fhg_add_test (we2/test/require_type.cpp
  PROJECT test_we
  LINK_LIBRARIES we2 we-dev fhg-util
)

fhg_add_test (we2/test/signature_of.cpp
  PROJECT test_we
  LINK_LIBRARIES we2 we-dev fhg-util
)

fhg_add_test (we2/test/exception.cpp
  PROJECT test_we
  LINK_LIBRARIES we2 we-dev fhg-util
)

add_executable (mk_cpp we2/type/test/mk_cpp.cpp)
target_link_libraries (mk_cpp we2 we-dev fhg-util ${Boost_LIBRARIES})

add_custom_command (
  OUTPUT sig.hpp sig.cpp
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/mk_cpp -H sig.hpp -I sig.cpp
  DEPENDS mk_cpp
)

include_directories (${CMAKE_CURRENT_BINARY_DIR})
add_library (sig ${CMAKE_CURRENT_BINARY_DIR}/sig.cpp)

fhg_add_test (we2/type/test/sig.cpp
  PROJECT test_we
  LINK_LIBRARIES sig we2 we-dev fhg-util
)
