# -*- mode: cmake; -*-
set (hdrs
  get_names.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/expr/parse/util
  COMPONENT headers)
