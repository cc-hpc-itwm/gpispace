# -*- mode: cmake; -*-
set (hdrs
  adjacency.hpp
  priostore.hpp
  svector.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/container
  COMPONENT headers)
