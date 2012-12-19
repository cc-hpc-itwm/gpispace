# -*- mode: cmake; -*-
set (hdrs
  adjacency.ipp
  adjacency.hpp
  bijection.hpp
  multirel.hpp
  priostore.hpp
  svector.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/container
  COMPONENT headers)
