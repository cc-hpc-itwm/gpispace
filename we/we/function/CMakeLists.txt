# -*- mode: cmake; -*-
set (hdrs
  cond.hpp
  trans.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/function
  COMPONENT headers)
