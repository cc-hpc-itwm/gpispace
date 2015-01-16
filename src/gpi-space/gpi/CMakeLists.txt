find_package (GASPI 1.1.0 REQUIRED)

find_package (IBVerbs REQUIRED)

add_library (gpi-space-gpiapi
  system.cpp
  fake_api.cpp
  gaspi.cpp
)
target_include_directories (gpi-space-gpiapi PRIVATE "${GASPI_INCLUDE_DIR}")
target_link_libraries (gpi-space-gpiapi ${GASPI_LIBRARY} ${IBVerbs_LIBRARY})
