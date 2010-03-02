# -*- mode: cmake; -*-

file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

include_directories(${PB_INCLUDE_DIR})
ADD_LIBRARY(sdpa-events ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa/events)
install(TARGETS sdpa-events ARCHIVE DESTINATION lib)
