# -*- mode: cmake; -*-
set (hdrs
  eq.hpp
  field.hpp
  function.hpp
  get.hpp
  hash.hpp
  mk_structured.hpp
  put.hpp
  require_type.hpp
  show.hpp
)

install(FILES ${hdrs}
  DESTINATION include/we/type/value
  COMPONENT headers)

add_subdirectory(cpp)
add_subdirectory(container)
