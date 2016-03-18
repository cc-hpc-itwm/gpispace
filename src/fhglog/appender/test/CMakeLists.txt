fhg_add_test (NAME fhglog_appender
  SOURCES appenders.cpp
  USE_BOOST
  LIBRARIES fhglog
)

fhg_add_test (NAME fhglog_appender_file
  SOURCES file.cpp
  USE_BOOST
  LIBRARIES fhglog
            fhg-util
            Boost::filesystem
)

fhg_add_test (NAME fhglog_appender_call
  SOURCES call.cpp
  USE_BOOST
  LIBRARIES fhglog
)
