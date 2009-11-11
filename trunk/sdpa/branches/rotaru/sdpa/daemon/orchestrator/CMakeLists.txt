# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB hdrs *.hpp)
file(GLOB srcs *.cpp)

ADD_LIBRARY(sdpa-orchestrator ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa/daemon/orchestrator)
install(TARGETS sdpa-orchestrator ARCHIVE DESTINATION lib)
