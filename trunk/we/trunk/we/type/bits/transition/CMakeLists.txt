# -*- mode: cmake; -*-
set (hdrs
  optimize.hpp
  toDot.hpp
)

install(FILES ${hdrs} DESTINATION include/we/type/bits/transition)

add_subdirectory (optimize)
