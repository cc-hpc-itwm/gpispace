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
)

file(GLOB events_src *.cpp *.hpp)

ADD_LIBRARY(sdpa-events ${events_src})

install(FILES ${hdrs} DESTINATION include/sdpa/events)
install(TARGETS sdpa-events ARCHIVE DESTINATION lib)

