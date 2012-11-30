# -*- mode: cmake; -*-

set (hdrs
  itopology.hpp
  topology.hpp
)

set (srcs
  topology.cpp
)

ADD_LIBRARY(gpi-space-pc-global ${hdrs} ${srcs})
target_link_libraries(gpi-space-pc-global
  ${FhgCom_LIBRARY}
)

install(TARGETS gpi-space-pc-global
  DESTINATION lib
  COMPONENT libraries)

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/global
  COMPONENT headers)
