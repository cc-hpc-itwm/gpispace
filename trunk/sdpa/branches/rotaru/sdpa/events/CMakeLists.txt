include_directories(.)

set(hdrs InternalEvent.hpp
ErrorEvent.hpp
JobEvent.hpp
SubmitJobEvent.hpp
RunJobEvent.hpp
CancelJobEvent.hpp
CancelJobAckEvent.hpp
FailJobEvent.hpp
JobFinishedEvent.hpp
QueryJobStatusEvent.hpp
JobStatusAnswerEvent.hpp
SDPAEvent.hpp
)
add_protocol_buffers(sdpa-msg)

SET_SOURCE_FILES_PROPERTIES(sdpa-events.pb.h PROPERTIES GENERATED 1)
SET_SOURCE_FILES_PROPERTIES(sdpa-events.pb.cc PROPERTIES GENERATED 1)

file(GLOB hdrs "*.hpp" "sdpa-msg.pb.h")
file(GLOB srcs "*.cpp" "sdpa-msg.pb.cc")

include_directories(${PB_INCLUDE_DIR})
ADD_LIBRARY(sdpa-events ${hdrs} ${srcs})
ADD_DEPENDENCIES(sdpa-events sdpa-msg.pb.h sdpa-msg.pb.cc)

install(FILES ${hdrs} DESTINATION include/sdpa/events)
install(TARGETS sdpa-events ARCHIVE DESTINATION lib)
