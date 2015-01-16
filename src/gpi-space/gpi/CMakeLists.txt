find_package (GASPI 1.1.0 REQUIRED)
include_directories(${GASPI_INCLUDE_DIR})

find_package (IBVerbs REQUIRED)

set (srcs
  system.cpp
  fake_api.cpp
  gaspi.cpp
)

add_library (gpi-space-gpiapi ${srcs})
target_link_libraries (gpi-space-gpiapi ${GASPI_LIBRARY} ${IBVerbs_LIBRARY})
