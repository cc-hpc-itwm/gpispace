# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

install(FILES ${hdrs} DESTINATION include/sdpa/daemon/aggregator)

