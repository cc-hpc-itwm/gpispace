# -*- mode: cmake; -*-
set (hdrs
  position.hpp
)

install(FILES ${hdrs}
  DESTINATION include/fhg/util/parse
  COMPONENT headers)
