# -*- mode: cmake; -*-

fhg_add_application_test (kvs_pressure
  SCRIPT test_kvs_pressure
)

set(tc_srcs
  test_store.cpp
  test_kvs_message.cpp
  test_global_kvs.cpp
  test_peer.cpp
  test_message.cpp
  test_connection.cpp

  test_p2p.cpp
)

foreach (tc_src ${tc_srcs})
  # get the filename without extension
  string(REGEX REPLACE "(.*/)?test_(.*)\\.c.*" "test_${PROJECT_NAME}_\\2" tc_name ${tc_src})
  message(STATUS "adding test case ${tc_name} (using ${tc_src})")

  add_executable(${tc_name} ${tc_src})
  target_link_libraries(${tc_name} fhgcom)
  target_link_libraries(${tc_name} ${FhgLog_LIBRARY})
  target_link_libraries(${tc_name} ${Boost_LIBRARIES})
  target_link_libraries(${tc_name} ${Boost_UNIT_TEST_LIBRARIES})
  get_target_property(LOC ${tc_name} LOCATION_)
  add_test(${tc_name} ${LOC})
endforeach(tc_src ${tc_srcs})

add_subdirectory(unix)
