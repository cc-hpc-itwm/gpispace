# -*- mode: cmake; -*-

set (hdrs
  types.hpp
  error.hpp
  exception.hpp
  signal_handler.hpp
  )

set (srcs
  signal_handler.cpp
  )

if (NO_IMPLICIT_TEMPLATES)
  set (srcs ${srcs} template.cpp)
endif (NO_IMPLICIT_TEMPLATES)

ADD_LIBRARY(gpi-space-static ${hdrs} ${srcs})
ADD_LIBRARY(gpi-space-shared SHARED ${hdrs} ${srcs})
set_target_properties(gpi-space-static PROPERTIES OUTPUT_NAME gpi-space)
set_target_properties(gpi-space-shared PROPERTIES OUTPUT_NAME gpi-space)

add_subdirectory(config)
add_subdirectory(gpi)
add_subdirectory(daemon)
add_subdirectory(pc)
add_subdirectory(shell)

install(FILES ${hdrs}
  DESTINATION include/gpi-space
  COMPONENT headers)

install(TARGETS gpi-space-static
  ARCHIVE
  DESTINATION lib
  COMPONENT static-libraries)

install(TARGETS gpi-space-shared
  LIBRARY
  DESTINATION lib
  COMPONENT libraries)
