# -*- mode: cmake; -*-

set (hdrs
  process.hpp
  message_visitor.hpp
  connector.hpp
  manager.hpp
  )

set (srcs
  manager.cpp
  )

if (GPI_SPACE_HEADER_ONLY)
else (GPI_SPACE_HEADER_ONLY)
  set (srcs ${srcs} templates.cpp)
endif (GPI_SPACE_HEADER_ONLY)

ADD_LIBRARY(gpi-space-pc-container ${hdrs} ${srcs})

install(TARGETS gpi-space-pc-container
  DESTINATION lib
  COMPONENT libraries)

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/container
  COMPONENT headers)
