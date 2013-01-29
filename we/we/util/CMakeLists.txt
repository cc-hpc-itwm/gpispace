# -*- mode: cmake; -*-
set (hdrs
  cross.hpp
  stat.hpp
  token.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/util
  COMPONENT headers)
