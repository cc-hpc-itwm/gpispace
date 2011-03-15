# -*- mode: cmake; -*-

set (hdrs
  memory.hpp
  segment.hpp
  error.hpp
  control.hpp
  message.hpp

  channel.hpp
  )

set (srcs
  channel.cpp
  )

add_library(gpi-space-pc-proto ${hdrs} ${srcs})

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/proto
  COMPONENT headers)
