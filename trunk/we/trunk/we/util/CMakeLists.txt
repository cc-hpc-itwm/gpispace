# -*- mode: cmake; -*-
set (hdrs
  codec.hpp
  cross.hpp
  it.hpp
  stat.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/util
  COMPONENT headers)
