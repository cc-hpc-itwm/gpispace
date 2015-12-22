ADD_LIBRARY(gpi-space-pc-memory
  memory_buffer.cpp
  memory_area.cpp
  handle_generator.cpp
  manager.cpp
  gaspi_area.cpp
  shm_area.cpp
  beegfs_area.cpp
)
target_include_directories (gpi-space-pc-memory
  SYSTEM PRIVATE "${LIBRT_INCLUDE_DIR}"
)
target_link_libraries (gpi-space-pc-memory
  gpi-space-pc-segment
  mmgr
  gpi-space_url
  Util::Generic
  gpi-space-gpiapi
  )
