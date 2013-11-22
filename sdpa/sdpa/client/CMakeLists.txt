# -*- mode: cmake; -*-

include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${CMAKE_CURRENT_BINARY_DIR})

set (srcs
  Client.cpp
  )

set (hdrs
  Client.hpp
  )

ADD_LIBRARY(sdpa-client ${hdrs} ${srcs})
