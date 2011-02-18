# -*- mode: cmake; -*-

set (hdrs
  command.hpp
  )

set (srcs
  command.cpp
  )

add_library(gpi-space-shell ${hdrs} ${srcs})

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/cmdline
  COMPONENT headers)
