set (hdrs
  types.hpp
  exception.hpp
  )

add_subdirectory(gpi)
add_subdirectory(pc)

install(FILES ${hdrs}
  DESTINATION include/gpi-space
  COMPONENT headers)
