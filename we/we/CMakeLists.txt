# -*- mode: cmake; -*-
add_subdirectory (container)
add_subdirectory (expr)
add_subdirectory (mgmt)
add_subdirectory (serialize)
add_subdirectory (type)
add_subdirectory (util)
add_subdirectory (loader)

add_library (we ${WE_SOURCES})
target_link_libraries (we
  fhg-util
  ${Boost_LIBRARIES}
)
set (WE_LIBRARIES we CACHE INTERNAL "we_libraries" FORCE)
