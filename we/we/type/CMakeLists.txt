# -*- mode: cmake; -*-
set (hdrs
  bitsetofint.hpp
  bytearray.hpp
  condition.hpp
  connection.hpp
  control.hpp
  error.hpp
  expression.hpp
  id.hpp
  literal.hpp
  module_call.hpp
  place.hpp
  port.hpp
  property.hpp
  signature.hpp
  token.hpp
  transition.hpp
  value.hpp
  requirement.hpp
  )
set (sources
  property.cpp
  port.cpp
  id.cpp
  place.cpp
  connection.cpp
  expression.cpp
)

install(FILES ${hdrs}
  DESTINATION include/we/type
  COMPONENT headers)

add_library (we_type ${sources})
add_we_library (we_type)
target_link_libraries (we_type
  fhg-util
  we_expr_parse_simplify
  ${Boost_LIBRARIES}
)

add_subdirectory (bits)
add_subdirectory (literal)
add_subdirectory (value)
add_subdirectory (signature)

if (BUILD_TESTING)
  add_subdirectory (test)
endif (BUILD_TESTING)
