# -*- mode: cmake; -*-

set (hdrs
  typedefs.hpp
  time_stamp.hpp
  generic_list.hpp
  segment_descriptor.hpp
  handle_descriptor.hpp
  process_descriptor.hpp
  info_descriptor.hpp
  memory_location.hpp
  handle.hpp
  counter.hpp
  )

set (srcs
  )

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/type
  COMPONENT headers)
