# -*- mode: cmake; -*-

set (hdrs
  memory_area.hpp
  handle_generator.hpp
  manager.hpp
  gpi_area.hpp
  shm_area.hpp
  )

set (srcs
  memory_area.cpp
  handle_generator.cpp
  manager.cpp
  gpi_area.cpp
  shm_area.cpp
  )

ADD_LIBRARY(gpi-space-pc-memory ${hdrs} ${srcs})
target_link_libraries (gpi-space-pc-memory
  gpi-space-pc-segment
  ${MMGR_LIBRARY}
  )

install(TARGETS gpi-space-pc-memory
  DESTINATION lib
  COMPONENT libraries)

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/memory
  COMPONENT headers)
