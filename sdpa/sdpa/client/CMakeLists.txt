# -*- mode: cmake; -*-
find_package(SMC QUIET)
if (SMC_FOUND)
    include_directories(SYSTEM ${SMC_INCLUDE_DIR})
endif (SMC_FOUND)
include (SMCMacros)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${CMAKE_CURRENT_BINARY_DIR})

set (srcs
  Client.cpp
  )

set (hdrs
  Client.hpp
  ClientEvents.hpp
  exceptions.hpp
  types.hpp
  job_info.hpp
  )

ADD_LIBRARY(sdpa-client ${hdrs} ${srcs})
