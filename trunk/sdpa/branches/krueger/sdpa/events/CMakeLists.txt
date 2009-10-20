# -*- mode: cmake; -*-

add_protocol_buffers(SDPA_MSG_PB_CPP SDPA_MSG_PB_H sdpa-msg)

file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

set(hdrs ${SDPA_MSG_PB_H} ${hdrs} )
set(srcs ${SDPA_MSG_PB_CPP} ${srcs} )

include_directories(${PB_INCLUDE_DIR})
ADD_LIBRARY(sdpa-events ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa/events)
install(TARGETS sdpa-events ARCHIVE DESTINATION lib)
