set (hdrs
  memory.hpp
  segment.hpp
  error.hpp
  control.hpp
  message.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/proto
  COMPONENT headers)
