# -*- mode: cmake; -*-

set (hdrs
  typedefs.hpp
  time_stamp.hpp
  generic_list.hpp
  segment_descriptor.hpp
  handle_descriptor.hpp
  info_descriptor.hpp
  memory_location.hpp
  handle.hpp
  counter.hpp
  )

set (srcs
  )

#ADD_LIBRARY(gpi-space-daemon-static ${hdrs} ${srcs})
#ADD_LIBRARY(gpi-space-daemon-shared SHARED ${hdrs} ${srcs})
#set_target_properties(gpi-space-daemon-static PROPERTIES OUTPUT_NAME gpi-space-daemon)
#set_target_properties(gpi-space-daemon-shared PROPERTIES OUTPUT_NAME gpi-space-daemon)

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/proto
  COMPONENT headers)

#install(TARGETS gpi-space-daemon-static
#  ARCHIVE
#  DESTINATION lib
#  COMPONENT libraries)
#
#install(TARGETS gpi-space-daemon-shared
#  LIBRARY
#  DESTINATION lib
#  COMPONENT libraries)
