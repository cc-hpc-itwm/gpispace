# -*- mode: cmake; -*-

set (hdrs
  cache.hpp
  store.hpp
  kvsd.hpp
)

add_subdirectory(message)

install(FILES ${hdrs} DESTINATION include/fhgcom/kvs)
