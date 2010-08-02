# -*- mode: cmake; -*-

file(GLOB tc_srcs "test_*.c" "test_*.cpp")
list(SORT tc_srcs)
foreach (tc_src ${tc_srcs})
  # get the filename without extension
  string(REGEX REPLACE "(.*/)?test_(.*)\\.c.*" "test_${PROJECT_NAME}_\\2" tc_name ${tc_src})
  message(STATUS "adding test case ${tc_name} (using ${tc_src})")

  add_executable(${tc_name} ${tc_src})
  target_link_libraries(${tc_name} ${FhgLog_LIBRARY})
  target_link_libraries(${tc_name} ${Boost_LIBRARIES})
  target_link_libraries(${tc_name} dl)
  get_target_property(LOC ${tc_name} LOCATION_)
  add_test(${tc_name} ${LOC})
endforeach(tc_src ${tc_srcs})

file(GLOB gen_srcs "generate_*.cpp")
list(SORT gen_srcs)
foreach (gen_src ${gen_srcs})
  # get the filename without extension
  string(REGEX REPLACE "(.*/)?(.*)\\.cpp.*" "\\2" gen_name ${gen_src})
  message(STATUS "adding generator ${gen_name} (using ${gen_src})")

  add_executable(${gen_name} ${gen_src})
  target_link_libraries(${gen_name} ${FhgLog_LIBRARY})
  target_link_libraries(${gen_name} ${Boost_LIBRARIES})
  get_target_property(LOC ${gen_name} LOCATION_)
endforeach(gen_src ${gen_srcs})

# answer module
add_library(answer SHARED answer.cpp)
add_library(question SHARED question.cpp)
