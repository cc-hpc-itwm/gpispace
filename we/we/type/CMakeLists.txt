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

install(FILES ${hdrs}
  DESTINATION include/we/type
  COMPONENT headers)

add_subdirectory (bits)
add_subdirectory (literal)
add_subdirectory (value)
add_subdirectory (signature)

if (BUILD_TESTING)
  add_subdirectory (test)
endif (BUILD_TESTING)
