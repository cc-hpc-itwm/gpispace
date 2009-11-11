# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB hdrs *.hpp)
file(GLOB srcs *.cpp)

ADD_LIBRARY(sdpa-nre ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa/daemon/nre)
install(TARGETS sdpa-nre ARCHIVE DESTINATION lib)
