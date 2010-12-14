# -*- mode: cmake; -*-
set (hdrs
  header_gen.hpp
  include.hpp
  include_guard.hpp
  constants.hpp
  types.hpp
)

install(FILES ${hdrs}
  DESTINATION include/fhg/util/cpp
  COMPONENT headers)
