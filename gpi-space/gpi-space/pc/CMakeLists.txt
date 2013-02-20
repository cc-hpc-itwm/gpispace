# -*- mode: cmake; -*-

add_subdirectory (type)
add_subdirectory (proto)
add_subdirectory (client)
add_subdirectory (segment)
add_subdirectory (memory)
add_subdirectory (container)
add_subdirectory (global)

add_subdirectory (plugin)

include(libutils)

merge_static_libs(gpi-space-pc gpi-space-pc "gpi-space-pc-client-static;gpi-space-pc-segment;gpi-space-static;${FhgLog_LIBRARY};${Boost_LIBRARIES};fhg-util")
merge_static_libs(gpi-space-pc-server
  gpi-space-pc-server
  "gpi-space-pc-container;gpi-space-pc-global;gpi-space-pc-memory;gpi-space-pc-segment;gpi-space-static;${MMGR_LIBRARY};${FhgCom_LIBRARY};${FhgLog_LIBRARY};${Boost_LIBRARIES};fhg-util"
)

install(TARGETS gpi-space-pc
  ARCHIVE
  DESTINATION lib
  COMPONENT static-libraries)
