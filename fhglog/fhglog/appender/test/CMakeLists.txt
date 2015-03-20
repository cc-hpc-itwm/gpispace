fhg_add_test (appenders.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog_appender
  LINK_LIBRARIES fhglog
)

fhg_add_test (file.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog_appender
  LINK_LIBRARIES fhglog
                 fhg-util
                 Boost::filesystem
)

fhg_add_test (call.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog_appender
  LINK_LIBRARIES fhglog
)
