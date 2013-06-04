# -*- mode: cmake; -*-

set (hdrs
  Appender.hpp
  color_map.hpp
  CompoundAppender.hpp
  Configuration.hpp
  DecoratingAppender.hpp
  DefaultConfiguration.hpp
  error_handler.hpp
  fhglog.hpp
  FileAppender.hpp
  MemoryAppender.hpp
  fileno.hpp
  Filter.hpp
  FilteringAppender.hpp
  format_flag.hpp
  format.hpp
  LogEvent.hpp
  LoggerApi.hpp
  logger_config.hpp
  Logger.hpp
  logger_path.hpp
  LogLevel.hpp
  LogMacros.hpp
  macros.hpp
  memory.hpp
  NullAppender.hpp
  StreamAppender.hpp
  SynchronizedAppender.hpp
  ThreadedAppender.hpp
  types.hpp
  util.hpp
  minimal.hpp
  )

set (srcs
  util.cpp
  DefaultConfiguration.cpp
  color_map.cpp
  error_handler.cpp
  FileAppender.cpp
  MemoryAppender.cpp
  fileno.cpp
  LogEvent.cpp
  Logger.cpp
  logger_path.cpp
  LogLevel.cpp
  StreamAppender.cpp
  )

if (HAVE_SYSLOG_H)
  set(hdrs ${hdrs} "nonportable/SyslogAppender.hpp")
  set(srcs ${srcs} "nonportable/SyslogAppender.cpp")
endif (HAVE_SYSLOG_H)

if (ENABLE_REMOTE_LOGGING)
  add_subdirectory(remote)
  set (remote_hdrs
    remote/LogServer.hpp
    remote/RemoteAppender.hpp
    )
  set (remote_srcs
    remote/LogServer.cpp
    remote/RemoteAppender.cpp
    )
  install(FILES ${remote_hdrs}
    DESTINATION include/fhglog/remote
    COMPONENT runtime-headers)
endif (ENABLE_REMOTE_LOGGING)

ADD_LIBRARY(fhglog ${hdrs} ${srcs} ${remote_srcs} ${remote_hdrs})
ADD_LIBRARY(fhglog-shared SHARED ${hdrs} ${srcs} ${remote_srcs} ${remote_hdrs})
set_target_properties (fhglog-shared
  PROPERTIES OUTPUT_NAME fhglog
             VERSION     1
)

install(FILES ${hdrs}
  DESTINATION include/fhglog
  COMPONENT runtime-headers)
install(TARGETS fhglog
  ARCHIVE
  DESTINATION lib
  COMPONENT runtime-libraries)
install(TARGETS fhglog-shared
  LIBRARY
  DESTINATION lib
  COMPONENT runtime-libraries)
