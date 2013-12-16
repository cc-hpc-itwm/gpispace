set (tests
   test_default_config.cpp
   test_file_appender.cpp
   test_filtering_appender.cpp
   test_format.cpp
   test_formatter.cpp
   test_logger_config.cpp
   test_logging.cpp
   test_path.cpp
   test_perf.cpp
   test_split.cpp
   test_synch_appender.cpp
   test_threaded.cpp
   test_thread_safety.cpp
   test_once.cpp
)

foreach (test ${tests})
  fhg_add_test (${test}
    PROJECT fhglog
    LINK_LIBRARIES fhglog
                   ${Boost_THREAD_LIBRARY} # default_config, logging, perf, synch_appender, threaded only
  )
endforeach()

fhg_add_test (test_append.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (test_compound.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (test_disabled_logging.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (test_error_handler.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (test_event.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

fhg_add_test (test_every_n.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog)

if (ENABLE_REMOTE_LOGGING)
  add_subdirectory(remote)
endif (ENABLE_REMOTE_LOGGING)

fhg_add_test (dump_event dump_event.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog
)
