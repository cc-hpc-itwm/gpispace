set (hdrs
  memory_buffer.hpp
  handle_buffer.hpp
  memory_area.hpp
  handle_generator.hpp
  manager.hpp
  gpi_area.hpp
  shm_area.hpp
  sfs_area.hpp
  memory_transfer_t.hpp
  task.hpp
  transfer_queue.hpp
  transfer_manager.hpp
  factory.hpp
  )

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
  factory.cpp
  )

ADD_LIBRARY(gpi-space-pc-memory ${hdrs} ${srcs})
target_link_libraries (gpi-space-pc-memory
  gpi-space-pc-segment
  ${MMGR_LIBRARY}
  )

install(TARGETS gpi-space-pc-memory
  DESTINATION lib
  COMPONENT libraries)

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/memory
  COMPONENT headers)
