add_library (gpi-space-pc-container
  manager.cpp
  process.cpp
  templates.cpp
)

install (TARGETS gpi-space-pc-container
  DESTINATION lib
  COMPONENT libraries
)

install (FILES
  process.hpp
  message_visitor.hpp
  connector.hpp
  manager.hpp
  DESTINATION include/gpi-space/pc/container
  COMPONENT headers
)
