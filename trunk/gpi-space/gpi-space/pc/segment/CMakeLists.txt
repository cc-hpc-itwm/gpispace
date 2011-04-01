# -*- mode: cmake; -*-

set (hdrs
  segment.hpp
  #  manager.hpp
  )

set (srcs
  segment.cpp
  #  manager.cpp
  )

ADD_LIBRARY(gpi-space-pc-segment ${hdrs} ${srcs})
add_library(gpi-space-pc-segment-shared SHARED ${hdrs} ${srcs})
set_target_properties(gpi-space-pc-segment-shared PROPERTIES OUTPUT_NAME gpi-space-pc-segment)

target_link_libraries (gpi-space-pc-segment rt)
target_link_libraries (gpi-space-pc-segment-shared rt)

install(TARGETS gpi-space-pc-segment
  ARCHIVE
  DESTINATION lib
  COMPONENT modules)

install(TARGETS gpi-space-pc-segment-shared
  LIBRARY
  DESTINATION lib
  COMPONENT modules)

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/segment
  COMPONENT headers)
