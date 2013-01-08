# -*- mode: cmake; -*-
set (hdrs
  codec.hpp
  cross.hpp
  it.hpp
  stat.hpp
  token.hpp
  )

add_we_sources (
  codec.cpp
)

install(FILES ${hdrs}
  DESTINATION include/we/util
  COMPONENT headers)
