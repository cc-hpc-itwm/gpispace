set (hdrs
  Appender.hpp
  Configuration.hpp
  Filter.hpp
  LogMacros.hpp
  Logger.hpp
  LoggerApi.hpp
  color_map.hpp
  event.hpp
  fhglog.hpp
  format.hpp
  level.hpp
  )
set (header_appender
  appender/file.hpp
  appender/filtering.hpp
  appender/memory.hpp
  appender/stream.hpp
)

set (srcs
  Configuration.cpp
  Logger.cpp
  color_map.cpp
  event.cpp
  level.cpp
  )
set (source_appender
  appender/file.cpp
  appender/memory.cpp
  appender/stream.cpp
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

ADD_LIBRARY (fhglog
  ${hdrs}
  ${header_appender}
  ${srcs}
  ${source_appender}
  ${remote_srcs}
  ${remote_hdrs}
)
ADD_LIBRARY (fhglog-shared SHARED
  ${hdrs}
  ${header_appender}
  ${srcs}
  ${source_appender}
  ${remote_srcs}
  ${remote_hdrs}
)
set_target_properties (fhglog-shared
  PROPERTIES OUTPUT_NAME fhglog
             VERSION     2
)

target_link_libraries (fhglog fhg-util ${Boost_FILESYSTEM_LIBRARY})
target_link_libraries (fhglog-shared fhg-util ${Boost_FILESYSTEM_LIBRARY})

install(FILES ${hdrs}
  DESTINATION include/fhglog
  COMPONENT runtime-headers)
install(FILES ${header_appender}
  DESTINATION include/fhglog/appender
  COMPONENT runtime-headers)
install(TARGETS fhglog
  ARCHIVE
  DESTINATION lib
  COMPONENT runtime-libraries)
install(TARGETS fhglog-shared
  LIBRARY
  DESTINATION lib
  COMPONENT runtime-libraries)

add_subdirectory (appender/test)
