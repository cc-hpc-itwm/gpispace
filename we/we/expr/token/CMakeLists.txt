# -*- mode: cmake; -*-
set (hdrs
  assoc.hpp
  prec.hpp
  prop.hpp
  tokenizer.hpp
  type.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/expr/token
  COMPONENT headers)
