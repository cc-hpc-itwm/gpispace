file(GLOB all_hdrs "*.hpp")
file(GLOB all_srcs "*.cpp")

set(hdrs "sdpa-msg.pb.h"  ${all_hdrs})
set(srcs "sdpa-msg.pb.cc" ${all_srcs})

add_protocol_buffers(sdpa-msg)

include_directories(${PB_INCLUDE_DIR})
ADD_LIBRARY(sdpa-events ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa/messages)
install(TARGETS sdpa-events ARCHIVE DESTINATION lib)
