# -*- mode: cmake; -*-
set (hdrs
  cpp.hpp
  default.hpp
  function.hpp
  hash.hpp
  name.hpp
  read.hpp
  require_type.hpp
  show.hpp
  valid_name.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/type/literal
  COMPONENT headers)
