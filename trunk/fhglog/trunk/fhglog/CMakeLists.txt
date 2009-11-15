# -*- mode: cmake; -*-
file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

if (HAVE_SYSLOG_H)
  set(hdrs ${hdrs} "nonportable/SyslogAppender.hpp")
  set(srcs ${srcs} "nonportable/SyslogAppender.cpp")
endif (HAVE_SYSLOG_H)

if (WITH_REMOTE_LOGGING)
  add_subdirectory(remote)
  file(GLOB remote_srcs "remote/*.cpp")
  file(GLOB remote_hdrs "remote/*.hpp")
  install(FILES ${remote_hdrs} DESTINATION include/fhglog/remote)
endif (WITH_REMOTE_LOGGING)

ADD_LIBRARY(fhglog ${hdrs} ${srcs} ${remote_srcs} ${remote_hdrs})
ADD_LIBRARY(fhglog-shared SHARED ${hdrs} ${srcs} ${remote_srcs} ${remote_hdrs})
set_target_properties(fhglog-shared PROPERTIES OUTPUT_NAME fhglog)

install(FILES ${hdrs} DESTINATION include/fhglog)
install(TARGETS fhglog DESTINATION lib EXPORT ${PROJECT_NAME}Exports)
install(TARGETS fhglog-shared DESTINATION lib EXPORT ${PROJECT_NAME}Exports)
