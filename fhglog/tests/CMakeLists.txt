fhg_add_test (appenders.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (compound_appender.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (disabled_logging.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (dump_event.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (error_handler.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (event.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (every_n.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (file_appender.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog ${Boost_FILESYSTEM_LIBRARY})

fhg_add_test (filtering_appender.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (format.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (once.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (path.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (performance.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (remote_logging.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (synchronized_appender.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (threaded_appender.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (thread_safety.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)
