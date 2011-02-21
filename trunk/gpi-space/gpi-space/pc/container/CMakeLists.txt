# -*- mode: cmake; -*-

set (hdrs
  process.hpp
  process.ipp
  message_visitor.hpp

  connector.hpp
  connector.ipp

  manager.hpp
  manager.ipp
  )

set (srcs
  )

install(FILES ${hdrs}
  DESTINATION include/gpi-space/pc/container
  COMPONENT headers)
