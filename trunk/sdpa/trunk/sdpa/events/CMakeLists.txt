file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

add_protocol_buffers(sdpa-msg)
set(hdrs ${hdrs} "sdpa-msg.pb.h")

include_directories(${PB_INCLUDE_DIR})
ADD_LIBRARY(sdpa-events ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa/events)
install(TARGETS sdpa-events ARCHIVE DESTINATION lib)
