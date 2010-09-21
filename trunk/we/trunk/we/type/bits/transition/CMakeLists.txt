# -*- mode: cmake; -*-
set (hdrs
  optimize.hpp
  toDot.hpp
)

install(FILES ${hdrs}
  DESTINATION include/we/type/bits/transition
  COMPONENT headers)

add_subdirectory (optimize)
