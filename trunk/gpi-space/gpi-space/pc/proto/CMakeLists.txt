# -*- mode: cmake; -*-

set (hdrs
  memory.hpp
  segment.hpp
  error.hpp
  control.hpp
  message.hpp
  )

set (srcs
  )

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/proto
  COMPONENT headers)
