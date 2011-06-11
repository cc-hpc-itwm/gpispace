# -*- mode: cmake; -*-

add_subdirectory (type)
add_subdirectory (proto)
add_subdirectory (client)
add_subdirectory (segment)
add_subdirectory (memory)
add_subdirectory (container)
add_subdirectory (global)

include(libutils)

add_library(gpi-space-c-api api.cpp)

find_library (LIBRT_LIBRARY
  NAMES librt.a
  PATH_SUFFIXES lib
  )
merge_static_libs(gpi-space-pc gpi-space-pc "gpi-space-c-api;gpi-space-pc-client-static;gpi-space-pc-segment;gpi-space-static;${FhgLog_LIBRARY};${Boost_LIBRARIES};${LIBRT_LIBRARY}")

install(FILES api.h
  DESTINATION include/gpi-space/pc
  COMPONENT headers)

install(TARGETS gpi-space-pc
  ARCHIVE
  DESTINATION lib
  COMPONENT libraries)
