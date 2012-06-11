# -*- mode: cmake; -*-
set (hdrs
  basic_layer.hpp
  context.hpp
  exception.hpp
  layer.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/mgmt
  COMPONENT headers)

add_subdirectory (bits)
add_subdirectory (type)
