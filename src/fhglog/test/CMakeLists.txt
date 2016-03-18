fhg_add_test (NAME fhglog_dump_event
  SOURCES dump_event.cpp
  USE_BOOST
  LIBRARIES fhglog
)

fhg_add_test (NAME fhglog_dump_event.performance
  SOURCES dump_event.performance.cpp
  USE_BOOST
  PERFORMANCE_TEST
  LIBRARIES fhglog
)

fhg_add_test (NAME fhglog_event
  SOURCES event.cpp
  USE_BOOST
  LIBRARIES fhglog
)

fhg_add_test (NAME fhglog_format
  SOURCES format.cpp
  USE_BOOST
  LIBRARIES fhglog
)

fhg_add_test (NAME fhglog_format.performance
  SOURCES format.performance.cpp
  USE_BOOST
  PERFORMANCE_TEST
  LIBRARIES fhglog
)

fhg_add_test (NAME fhglog_remote_logging
  SOURCES remote_logging.cpp
  USE_BOOST
  LIBRARIES fhglog
)

fhg_add_test (NAME fhglog_level
  SOURCES level.cpp
  USE_BOOST
  LIBRARIES fhglog
)

fhg_add_test (NAME fhglog_thread_safety
  SOURCES thread_safety.cpp
  USE_BOOST
  LIBRARIES fhglog
)
