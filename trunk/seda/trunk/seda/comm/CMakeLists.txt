# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB hdrs "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
file(GLOB srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

ADD_LIBRARY(seda-comm ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/seda/comm)
install(TARGETS seda-comm 
	ARCHIVE DESTINATION lib)
