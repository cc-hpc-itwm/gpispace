set (srcs
  Configuration.cpp
  Logger.cpp
  event.cpp
  level.cpp
  appender/file.cpp
  appender/stream.cpp
  remote/server.cpp
  remote/appender.cpp
)

ADD_LIBRARY (fhglog ${srcs})

target_link_libraries (fhglog fhg-util ${Boost_FILESYSTEM_LIBRARY})

install (FILES
  Appender.hpp
  Configuration.hpp
  LogMacros.hpp
  Logger.hpp
  event.hpp
  level.hpp
  DESTINATION include/fhglog
)

install (TARGETS fhglog
  ARCHIVE
  DESTINATION lib
)

add_subdirectory (appender/test)
