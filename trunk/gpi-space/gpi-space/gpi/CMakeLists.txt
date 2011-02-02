# -*- mode: cmake; -*-

set (hdrs
  api.hpp
  )

set (srcs
  )

set (real_src
  ${srcs}
  real_api.cpp
  )

set (fake_src
  ${srcs}
  fake_api.cpp
  )

add_library (gpi-space-gpiapi      ${real_src} ${hdrs})
#add_library (gpi-space-server.fake ${srcs} ${fake_src} ${hdrs})
target_link_libraries (gpi-space-gpiapi ${GPI_LIBRARIES})

install(FILES ${hdrs}
  DESTINATION include/gpi-space/gpi
  COMPONENT headers)

install(TARGETS gpi-space-gpiapi
  ARCHIVE
  DESTINATION lib
  COMPONENT libraries)
