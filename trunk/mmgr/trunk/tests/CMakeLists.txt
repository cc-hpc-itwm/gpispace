# -*- mode: cmake; -*-

set (tc_srcs
  test_dtmmgr.c
  test_fseg.c
  test_heap.c
  test_ostab.c
  test_smap.c
  test_tmmgr.c
  test_trie.c
  )
list(SORT tc_srcs)

foreach (tc_src ${tc_srcs})
  # get the filename without extension
  string(REGEX REPLACE "(.*/)?test_(.*)\\.c.*" "test_${PROJECT_NAME}_\\2" tc_name ${tc_src})
  message(STATUS "adding test case ${tc_name} (using ${tc_src})")

  add_executable(${tc_name} ${tc_src})
  target_link_libraries(${tc_name} mmgr)
  get_target_property(LOC ${tc_name} LOCATION_)
  add_test(${tc_name} ${LOC})
endforeach(tc_src ${tc_srcs})
