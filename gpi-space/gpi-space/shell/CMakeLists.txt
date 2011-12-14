# -*- mode: cmake; -*-

set (hdrs
  shell.hpp
  shell.ipp
  command.hpp
  )

set (srcs
  )

#add_library(gpi-space-shell ${hdrs} ${srcs})

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/cmdline
  COMPONENT headers)
