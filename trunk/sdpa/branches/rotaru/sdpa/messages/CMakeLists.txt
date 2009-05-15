add_protocol_buffers(sdpa-msg)

SET_SOURCE_FILES_PROPERTIES(sdpa-msg.pb.h PROPERTIES GENERATED 1)
SET_SOURCE_FILES_PROPERTIES(sdpa-msg.pb.cc PROPERTIES GENERATED 1)

file(GLOB hdrs "*.hpp" "sdpa-msg.pb.h")
file(GLOB srcs "*.cpp" "sdpa-msg.pb.cc")

include_directories(${PB_INCLUDE_DIR})
ADD_LIBRARY(sdpa-messages ${hdrs} ${srcs})
ADD_DEPENDENCIES(sdpa-messages sdpa-msg.pb.h sdpa-msg.pb.cc)

install(FILES ${hdrs} DESTINATION include/sdpa/messages)
install(TARGETS sdpa-messages ARCHIVE DESTINATION lib)
