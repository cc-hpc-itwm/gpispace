# -*- mode: cmake; -*-

if(ENABLE_REAL_GPI)
  include_directories(${GPI_INCLUDE_DIR})
endif(ENABLE_REAL_GPI)

set (hdrs
  api.hpp
)

set (srcs
  api.cpp
  fake_api.cpp
)

if(ENABLE_REAL_GPI)
  set (srcs ${srcs} real_api.cpp)
endif(ENABLE_REAL_GPI)

add_library (gpi-space-gpiapi ${srcs})
if(ENABLE_REAL_GPI)
  target_link_libraries (gpi-space-gpiapi ${GPI_LIBRARIES})
endif(ENABLE_REAL_GPI)

install(FILES ${hdrs}
  DESTINATION include/gpi-space/gpi
  COMPONENT headers)

install(TARGETS gpi-space-gpiapi
  ARCHIVE
  DESTINATION lib
  COMPONENT libraries)
