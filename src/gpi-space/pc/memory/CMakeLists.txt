set (srcs
  memory_buffer.cpp
  memory_area.cpp
  handle_generator.cpp
  manager.cpp
  gpi_area.cpp
  shm_area.cpp
  sfs_area.cpp
  task.cpp
  transfer_queue.cpp
  transfer_manager.cpp
  )

ADD_LIBRARY(gpi-space-pc-memory ${srcs})
target_link_libraries (gpi-space-pc-memory
  gpi-space-pc-segment
  ${MMGR_LIBRARY}
  gpi-space_url
  )
