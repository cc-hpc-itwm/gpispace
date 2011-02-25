# -*- mode: cmake; -*-

set (hdrs
  process.hpp
  process.ipp
  message_visitor.hpp

  connector.hpp
  connector.ipp

  manager.hpp
  )

set (srcs
  manager.cpp
  )

ADD_LIBRARY(gpi-space-pc-container ${hdrs} ${srcs})
target_link_libraries (gpi-space-pc-container
  gpi-space-pc-segment
  gpi-space-pc-memory
  )

install(TARGETS gpi-space-pc-container
  DESTINATION lib
  COMPONENT libraries)

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/container
  COMPONENT headers)
