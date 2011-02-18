# -*- mode: cmake; -*-

set (hdrs
  memory_area.hpp
  )

set (srcs
  memory_area.cpp
  )

ADD_LIBRARY(gpi-space-pc-memory ${hdrs} ${srcs})
target_link_libraries (gpi-space-pc-memory rt)

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/segment
  COMPONENT headers)
