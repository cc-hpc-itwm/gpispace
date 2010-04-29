# -*- mode: cmake; -*-

file(GLOB tc_srcs "*.cpp")
list(SORT tc_srcs)
foreach (tc_src ${tc_srcs})
  # get the filename without extension
  string(REGEX REPLACE "(.*/)?(.*)\\.cpp.*" "\\2" tc_name ${tc_src})
  message(STATUS "adding ${tc_name} (using ${tc_src})")

  add_executable(${tc_name} ${tc_src})
  target_link_libraries(${tc_name} ${Boost_LIBRARIES})
  get_target_property(LOC ${tc_name} LOCATION_)
endforeach(tc_src ${tc_srcs})
