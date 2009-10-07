file(GLOB hdrs "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
file(GLOB srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_protocol_buffers(seda-msg)
list(APPEND srcs "${CMAKE_CURRENT_SOURCE_DIR}/seda-msg.pb.cc")
list(APPEND hdrs "${CMAKE_CURRENT_SOURCE_DIR}/seda-msg.pb.h")


ADD_LIBRARY(sedaComm ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/seda/comm)
install(TARGETS sedaComm 
	ARCHIVE DESTINATION lib)
