# -*- mode: cmake; -*-
set (hdrs
  types.hpp
  cpp.hpp
)
install(FILES ${hdrs}
  DESTINATION include/we/type/signature
  COMPONENT headers)
