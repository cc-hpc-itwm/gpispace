# -*- mode: cmake; -*-
set (hdrs
  assoc.hpp
  prec.hpp
  prop.hpp
  tokenizer.hpp
  type.hpp
  )

add_we_sources (
  assoc.cpp
  prec.cpp
  prop.cpp
  tokenizer.cpp
  type.cpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/expr/token
  COMPONENT headers)
