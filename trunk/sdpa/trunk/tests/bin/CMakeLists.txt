# -*- mode: cmake; -*-

file(GLOB tc_srcs "test_*.cpp")
list(SORT tc_srcs)
foreach (tc_src ${tc_srcs})
  # get the filename without extension
  string(REGEX REPLACE "(.*/)?(.*)\\.cpp.*" "\\2" tc_name ${tc_src})
  message(STATUS "adding test case ${tc_name} (using ${tc_src})")

  add_executable(${tc_name} ${tc_src})
  target_link_libraries(${tc_name} sdpa)
  target_link_libraries(${tc_name} sdpa-modules)
  target_link_libraries(${tc_name} sdpa-daemon)
  target_link_libraries(${tc_name} sdpa-events)
  target_link_libraries(${tc_name} sdpa-util)
  target_link_libraries(${tc_name} seda)
  target_link_libraries(${tc_name} seda-comm)
  get_target_property(LOC ${tc_name} LOCATION_)
  add_test(${tc_name} ${LOC})
endforeach(tc_src ${tc_srcs})
