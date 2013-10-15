set (tests
   test_append.cpp
   test_compound.cpp
   test_default_config.cpp
   test_disabled_logging.cpp
   test_error_handler.cpp
   test_event.cpp
   test_every_n.cpp
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
    LINK_LIBRARIES fhglog ${Boost_UNIT_TEST_LIBRARIES} ${Boost_LIBRARIES}
  )
endforeach()

if (ENABLE_REMOTE_LOGGING)
  add_subdirectory(remote)
endif (ENABLE_REMOTE_LOGGING)

fhg_add_test (dump_event dump_event.cpp
  PROJECT fhglog
  LINK_LIBRARIES fhglog
)
