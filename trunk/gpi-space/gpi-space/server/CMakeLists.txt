# -*- mode: cmake; -*-

set (hdrs
  gpi.hpp
  )

set (srcs
  gpi.cpp
  )

add_library (gpi-space-server ${srcs} ${hdrs})

install(FILES ${hdrs}
  DESTINATION include/gpi-space/config
  COMPONENT headers)
