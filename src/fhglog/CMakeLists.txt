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

target_link_libraries (fhglog fhg-util Boost::filesystem Util::Generic)

add_subdirectory (appender/test)
add_subdirectory (test)
