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
  we/field.cpp
  we/require_type.cpp
  we/signature_of.cpp
  we/type/signature/complete.cpp
  we/type/signature/cpp.cpp
  we/type/signature/dump.cpp
  we/type/signature/is_literal.cpp
  we/type/signature/name.cpp
  we/type/signature/show.cpp
  we/type/signature/signature.cpp
  we/type/value/name.cpp
  we/type/value/name_of.cpp
  we/type/value/of_type.cpp
  we/type/value/path/append.cpp
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

fhg_add_test (we/test/signature_of.cpp
  PROJECT test_we
  LINK_LIBRARIES we we-dev fhg-util
)

fhg_add_test (we/test/exception.cpp
  PROJECT test_we
  LINK_LIBRARIES we we-dev fhg-util
)

add_executable (mk_cpp we/type/test/mk_cpp.cpp)
target_link_libraries (mk_cpp we we-dev fhg-util ${Boost_LIBRARIES})

add_custom_command (
  OUTPUT sig.hpp sig.cpp
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/mk_cpp -H sig.hpp -I sig.cpp
  MAIN_DEPENDENCY mk_cpp
)

include_directories (${CMAKE_CURRENT_BINARY_DIR})
add_library (sig ${CMAKE_CURRENT_BINARY_DIR}/sig.cpp)

fhg_add_test (we/type/test/sig.cpp
  PROJECT test_we
  LINK_LIBRARIES sig we we-dev fhg-util
)
