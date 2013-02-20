# -*- mode: cmake; -*-

set (tc_srcs
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

list(SORT tc_srcs)

foreach (tc_src ${tc_srcs})
  # get the filename without extension
  string(REGEX REPLACE "(.*/)?test_(.*)\\.c.*" "test_${PROJECT_NAME}_\\2" tc_name ${tc_src})
  message(STATUS "adding test case ${tc_name} (using ${tc_src})")

  add_executable(${tc_name} ${tc_src})
  target_link_libraries(${tc_name} fhglog)
  target_link_libraries(${tc_name} ${Boost_UNIT_TEST_LIBRARIES})
  target_link_libraries(${tc_name} ${Boost_LIBRARIES})
  get_target_property(LOC ${tc_name} LOCATION_)
  add_test(${tc_name} ${LOC})
endforeach(tc_src ${tc_srcs})

if (ENABLE_REMOTE_LOGGING)
  add_subdirectory(remote)
endif (ENABLE_REMOTE_LOGGING)
