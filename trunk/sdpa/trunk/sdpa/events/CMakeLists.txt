file(GLOB all_hdrs "*.hpp")
file(GLOB all_srcs  "*.cpp")

set(hdrs "sdpa-msg.pb.h" ${all_hdrs})
set(srcs "sdpa-msg.pb.cc" ${all_srcs})

#SET_SOURCE_FILES_PROPERTIES(sdpa-msg.pb.h PROPERTIES GENERATED 1)
#SET_SOURCE_FILES_PROPERTIES(sdpa-msg.pb.cc PROPERTIES GENERATED 1)

add_protocol_buffers(sdpa-msg)

include_directories(${PB_INCLUDE_DIR})
ADD_LIBRARY(sdpa-messages ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa/messages)
install(TARGETS sdpa-messages ARCHIVE DESTINATION lib)
