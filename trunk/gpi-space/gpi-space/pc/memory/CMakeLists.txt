# -*- mode: cmake; -*-

set (hdrs
  memory_area.hpp
  handle_generator.hpp
  )

set (srcs
  memory_area.cpp
  handle_generator.cpp
  )

ADD_LIBRARY(gpi-space-pc-memory ${hdrs} ${srcs})
target_link_libraries (gpi-space-pc-memory
  gpi-space-pc-segment
  ${MMGR_LIBRARY}
  )

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/segment
  COMPONENT headers)
