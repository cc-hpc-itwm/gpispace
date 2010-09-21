# -*- mode: cmake; -*-

set (hdrs
  to_hex.hpp
)

install(FILES ${hdrs}
  DESTINATION include/fhgcom/util
  COMPONENT headers)
