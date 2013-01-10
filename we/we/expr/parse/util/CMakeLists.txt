# -*- mode: cmake; -*-
set (hdrs
  get_names.hpp
  )

add_we_sources (
  get_names.cpp
)

install(FILES ${hdrs}
  DESTINATION include/we/expr/parse/util
  COMPONENT headers)
