# -*- mode: cmake; -*-

set (hdrs
  node_config.hpp
  node_config_io.hpp
  node_config_io.tpp
  )

set (srcs
  )

install(FILES ${hdrs}
  DESTINATION include/gpi-space/config
  COMPONENT headers)
