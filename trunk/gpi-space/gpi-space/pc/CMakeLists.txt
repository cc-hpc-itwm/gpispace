# -*- mode: cmake; -*-

add_subdirectory (type)
add_subdirectory (proto)
add_subdirectory (client)
add_subdirectory (segment)
add_subdirectory (memory)
add_subdirectory (container)
add_subdirectory (global)

include(libutils)

find_library (LIBRT_LIBRARY
  NAMES librt.a
  PATH_SUFFIXES lib
  )
merge_static_libs(gpi-space-pc gpi-space-pc "gpi-space-pc-client-static;gpi-space-pc-segment;gpi-space-static;${FhgLog_LIBRARY};${Boost_LIBRARIES};${LIBRT_LIBRARY}")

install(TARGETS gpi-space-pc
  ARCHIVE
  DESTINATION lib
  COMPONENT libraries)
