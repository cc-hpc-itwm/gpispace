# -*- mode: cmake; -*-
set (hdrs
  stat.hpp
  token.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/util
  COMPONENT headers)
