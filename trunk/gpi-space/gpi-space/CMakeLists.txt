# -*- mode: cmake; -*-

set (hdrs
  gpi-space.hpp
  types.hpp
  error.hpp
  )

set (srcs
  gpi-space.cpp
  )

if (NO_IMPLICIT_TEMPLATES)
  set (srcs ${srcs} template.cpp)
endif (NO_IMPLICIT_TEMPLATES)

add_subdirectory(config)
add_subdirectory(server)

ADD_LIBRARY(gpi-space-static ${hdrs} ${srcs})
#ADD_LIBRARY(gpi-space-shared SHARED ${hdrs} ${srcs})
set_target_properties(gpi-space-static PROPERTIES OUTPUT_NAME gpi-space)
#set_target_properties(gpi-space-shared PROPERTIES OUTPUT_NAME gpi-space)

install(FILES ${hdrs}
  DESTINATION include/gpi-space
  COMPONENT headers)

install(TARGETS gpi-space-static
  ARCHIVE
  DESTINATION lib
  COMPONENT libraries)

#install(TARGETS gpi-space-shared
#  LIBRARY
#  DESTINATION lib
#  COMPONENT libraries)
