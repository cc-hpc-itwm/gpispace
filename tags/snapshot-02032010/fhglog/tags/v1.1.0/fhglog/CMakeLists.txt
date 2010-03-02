# -*- mode: cmake; -*-
file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

if (HAVE_SYSLOG_H)
  set(hdrs ${hdrs} "nonportable/SyslogAppender.hpp")
  set(srcs ${srcs} "nonportable/SyslogAppender.cpp")
endif (HAVE_SYSLOG_H)

if (WITH_REMOTE_LOGGING)
  add_subdirectory(remote)
endif (WITH_REMOTE_LOGGING)

ADD_LIBRARY(fhglog ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/fhglog)
install(TARGETS fhglog DESTINATION lib
  EXPORT ${PROJECT_NAME}Exports
)
