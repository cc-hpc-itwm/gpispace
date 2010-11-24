# -*- mode: cmake; -*-

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/ini)

set (hdrs
  node_config.hpp
  node_config_io.hpp
  node_config_io.tpp
  logging.hpp

  ini/ini.h
  )

set (srcs
  ini/ini.c
  )

install(FILES ${hdrs}
  DESTINATION include/gpi-space/config
  COMPONENT headers)
