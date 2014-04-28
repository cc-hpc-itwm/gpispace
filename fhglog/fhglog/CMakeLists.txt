set (hdrs
  Appender.hpp
  Configuration.hpp
  LogMacros.hpp
  Logger.hpp
  event.hpp
  fhglog.hpp
  level.hpp
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
  appender/stream.cpp
)

set (remote_srcs
  remote/server.cpp
  remote/appender.cpp
  )

ADD_LIBRARY (fhglog
  ${srcs}
  ${source_appender}
  ${remote_srcs}
)
ADD_LIBRARY (fhglog-shared SHARED
  ${srcs}
  ${source_appender}
  ${remote_srcs}
)
set_target_properties (fhglog-shared
  PROPERTIES OUTPUT_NAME fhglog
             VERSION     2
)

target_link_libraries (fhglog fhg-util ${Boost_FILESYSTEM_LIBRARY})
target_link_libraries (fhglog-shared fhg-util ${Boost_FILESYSTEM_LIBRARY})

install(FILES ${hdrs}
  DESTINATION include/fhglog
)
install(TARGETS fhglog
  ARCHIVE
  DESTINATION lib
)
install(TARGETS fhglog-shared
  LIBRARY
  DESTINATION lib
)

add_subdirectory (appender/test)
