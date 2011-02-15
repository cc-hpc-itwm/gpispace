# -*- mode: cmake; -*-

add_subdirectory (type)
add_subdirectory (proto)
add_subdirectory (client)
add_subdirectory (container)
add_subdirectory (segment)
add_subdirectory (cmdline)

set (hdrs
  )

set (srcs
  )

#ADD_LIBRARY(gpi-space-daemon-static ${hdrs} ${srcs})
#ADD_LIBRARY(gpi-space-daemon-shared SHARED ${hdrs} ${srcs})
#set_target_properties(gpi-space-daemon-static PROPERTIES OUTPUT_NAME gpi-space-daemon)
#set_target_properties(gpi-space-daemon-shared PROPERTIES OUTPUT_NAME gpi-space-daemon)

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc
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
