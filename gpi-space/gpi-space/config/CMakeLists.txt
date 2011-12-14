# -*- mode: cmake; -*-

set (hdrs
  config-data.hpp
  config.hpp
  config_io.hpp
  config_io.tpp
  node.hpp
  gpi.hpp
  logging.hpp
  )

set (srcs
  )

install(FILES ${hdrs}
  DESTINATION include/gpi-space/config
  COMPONENT headers)
