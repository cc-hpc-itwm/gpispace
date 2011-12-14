# -*- mode: cmake; -*-
set (hdrs
  validprefix.hpp
  )
install(FILES ${hdrs}
  DESTINATION include/rewrite
  COMPONENT headers)
