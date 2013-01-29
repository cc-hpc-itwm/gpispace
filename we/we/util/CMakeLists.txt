# -*- mode: cmake; -*-
set (hdrs
  cross.hpp
  stat.hpp
  token.hpp
  )

add_we_sources (cross.cpp)

install(FILES ${hdrs}
  DESTINATION include/we/util
  COMPONENT headers)
