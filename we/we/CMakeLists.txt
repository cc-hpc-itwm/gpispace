# -*- mode: cmake; -*-
set (hdrs
  net.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we
  COMPONENT headers)

add_subdirectory (container)
add_subdirectory (expr)
add_subdirectory (mgmt)
add_subdirectory (serialize)
add_subdirectory (type)
add_subdirectory (util)
add_subdirectory (loader)
