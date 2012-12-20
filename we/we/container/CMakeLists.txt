# -*- mode: cmake; -*-
set (hdrs
  adjacency.ipp
  adjacency.hpp
  multirel.hpp
  priostore.hpp
  svector.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/container
  COMPONENT headers)
