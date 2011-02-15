# -*- mode: cmake; -*-

set (hdrs
  command.hpp
  )

set (srcs
  command.cpp
  )

add_library(gpi-space-pc-cmdline ${hdrs} ${srcs})

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/cmdline
  COMPONENT headers)
