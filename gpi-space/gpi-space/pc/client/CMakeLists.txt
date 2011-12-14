# -*- mode: cmake; -*-

set (hdrs
  api.hpp
  )

set (srcs
  api.cpp
  )

add_library(gpi-space-pc-client-static ${hdrs} ${srcs})
add_library(gpi-space-pc-client-shared SHARED ${hdrs} ${srcs})
set_target_properties(gpi-space-pc-client-static PROPERTIES OUTPUT_NAME gpi-space-pc-client)
set_target_properties(gpi-space-pc-client-shared PROPERTIES OUTPUT_NAME gpi-space-pc-client)

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/client
  COMPONENT headers)

install(TARGETS gpi-space-pc-client-static
  ARCHIVE
  DESTINATION lib
  COMPONENT libraries)

install(TARGETS gpi-space-pc-client-shared
  LIBRARY
  DESTINATION lib
  COMPONENT libraries)
