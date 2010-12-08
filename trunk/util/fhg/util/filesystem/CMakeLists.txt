# -*- mode: cmake; -*-
set (hdrs
  mkdir.hpp
)

install(FILES ${hdrs}
  DESTINATION include/fhg/util/filesystem
  COMPONENT headers)
