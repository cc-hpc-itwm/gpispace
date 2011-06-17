# -*- mode: cmake; -*-

set(tc_srcs
  test_config.cpp
  test_signal_handler.cpp
  test_signal_handler_standalone.cpp
  test_pc_types.cpp
  test_segment.cpp
  test_handle_generator.cpp
  test_memory_manager.cpp
  test_task.cpp
)

foreach (tc_src ${tc_srcs})
  # get the filename without extension
  string(REGEX REPLACE "(.*/)?test_(.*)\\.c.*" "test_${PROJECT_NAME}_\\2" tc_name ${tc_src})
  message(STATUS "adding test case ${tc_name} (using ${tc_src})")

  add_executable(${tc_name} ${tc_src})
  target_link_libraries(${tc_name} gpi-space-pc-segment)
  target_link_libraries(${tc_name} gpi-space-pc-memory)
  target_link_libraries(${tc_name} gpi-space-static)
  target_link_libraries(${tc_name} ${FhgCom_LIBRARY})
  target_link_libraries(${tc_name} ${FhgLog_LIBRARY})
  target_link_libraries(${tc_name} ${Boost_UNIT_TEST_LIBRARIES})
  target_link_libraries(${tc_name} ${Boost_LIBRARIES})
  target_link_libraries(${tc_name} ${CMAKE_THREAD_LIBS_INIT})
  get_target_property(LOC ${tc_name} LOCATION_)
  add_test(${tc_name} ${LOC})
endforeach(tc_src ${tc_srcs})
