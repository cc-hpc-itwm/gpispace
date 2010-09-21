# -*- mode: cmake; -*-
set (hdrs
  join.hpp
  maybe.hpp
  read_bool.hpp
  read.hpp
  show.hpp
  split.hpp
  starts_with.hpp
  util.hpp
  warnings.hpp
)

install(FILES ${hdrs}
  DESTINATION include/fhg/util
  COMPONENT headers)
