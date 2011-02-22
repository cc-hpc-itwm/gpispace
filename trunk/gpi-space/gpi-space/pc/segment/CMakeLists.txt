# -*- mode: cmake; -*-

set (hdrs
  segment.hpp
  #  manager.hpp
  )

set (srcs
  segment.cpp
  #  manager.cpp
  )

ADD_LIBRARY(gpi-space-pc-segment ${hdrs} ${srcs})
target_link_libraries (gpi-space-pc-segment rt)

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/segment
  COMPONENT headers)
