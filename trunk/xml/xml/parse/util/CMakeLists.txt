set (hdrs
  error.hpp
  parser.cpp
  parser.hpp
  state.hpp
  types.hpp
  util.hpp
  warning.hpp
  )

install (FILES ${hdrs}
  DESTINATION include/xml/parse/util
  COMPONENT headers)
