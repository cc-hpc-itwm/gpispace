# -*- mode: cmake; -*-

set (hdrs
  gpi.hpp
  )

set (srcs
  )

set (real_src
  ${srcs}
  gpi_real.cpp
  )

set (fake_src
  ${srcs}
  gpi_fake.cpp
  )

add_library (gpi-space-server      ${real_src} ${hdrs})
#add_library (gpi-space-server.fake ${srcs} ${fake_src} ${hdrs})
target_link_libraries (gpi-space-server ${GPI_LIBRARIES})

install(FILES ${hdrs}
  DESTINATION include/gpi-space/config
  COMPONENT headers)

install(TARGETS gpi-space-server
  ARCHIVE
  DESTINATION lib
  COMPONENT libraries)
