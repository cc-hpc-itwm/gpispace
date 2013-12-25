# -*- mode: cmake; -*-

set (hdrs
  Appender.hpp
  color_map.hpp
  CompoundAppender.hpp
  Configuration.hpp
  DecoratingAppender.hpp
  fhglog.hpp
  FileAppender.hpp
  MemoryAppender.hpp
  Filter.hpp
  FilteringAppender.hpp
  format_flag.hpp
  format.hpp
  event.hpp
  LoggerApi.hpp
  Logger.hpp
  level.hpp
  LogMacros.hpp
  NullAppender.hpp
  StreamAppender.hpp
  SynchronizedAppender.hpp
  ThreadedAppender.hpp
  )

set (srcs
  Configuration.cpp
  color_map.cpp
  FileAppender.cpp
  MemoryAppender.cpp
  event.cpp
  Logger.cpp
  level.cpp
  StreamAppender.cpp
  )

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

ADD_LIBRARY(fhglog ${hdrs} ${srcs} ${remote_srcs} ${remote_hdrs})
ADD_LIBRARY(fhglog-shared SHARED ${hdrs} ${srcs} ${remote_srcs} ${remote_hdrs})
set_target_properties (fhglog-shared
  PROPERTIES OUTPUT_NAME fhglog
             VERSION     2
)

target_link_libraries (fhglog fhg-util ${Boost_FILESYSTEM_LIBRARY})
target_link_libraries (fhglog-shared fhg-util ${Boost_FILESYSTEM_LIBRARY})

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
