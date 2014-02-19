add_library (gpi-space-pc-container
  manager.cpp
  process.cpp
)

install (TARGETS gpi-space-pc-container
  DESTINATION lib
  COMPONENT libraries
)
