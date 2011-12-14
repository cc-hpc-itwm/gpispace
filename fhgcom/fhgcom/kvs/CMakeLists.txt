# -*- mode: cmake; -*-

set (hdrs
  backend.hpp
  cache.hpp
  store.hpp
  exception.hpp

  kvsc.hpp
  kvsd.hpp
)

add_subdirectory(message)

install(FILES ${hdrs}
  DESTINATION include/fhgcom/kvs
  COMPONENT headers)
