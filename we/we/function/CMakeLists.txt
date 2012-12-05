# -*- mode: cmake; -*-
set (hdrs
  cond.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/function
  COMPONENT headers)
