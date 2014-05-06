include_directories(${GPI_INCLUDE_DIR})

set (hdrs
  system.hpp
  api.hpp
)

set (srcs
  system.cpp
  fake_api.cpp
  real_api.cpp
)

add_library (gpi-space-gpiapi ${srcs})
target_link_libraries (gpi-space-gpiapi ${GPI_LIBRARIES})
