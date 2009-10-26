# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_protocol_buffers(SEDA_MSG_PB_CPP SEDA_MSG_PB_H seda-msg)

file(GLOB hdrs "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
file(GLOB srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

set(hdrs ${SEDA_MSG_PB_H} ${hdrs})
set(srcs ${SEDA_MSG_PB_CPP} ${srcs})

ADD_LIBRARY(seda-comm ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/seda/comm)
install(TARGETS seda-comm 
	ARCHIVE DESTINATION lib)
