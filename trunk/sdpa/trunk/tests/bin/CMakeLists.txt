# -*- mode: cmake; -*-
set (tc_srcs
  test_codec.cpp
  test_notification.cpp

  test_network_strategy.cpp
  )
list(SORT tc_srcs)

foreach (tc_src ${tc_srcs})
  # get the filename without extension
  string(REGEX REPLACE "(.*/)?test_(.*)\\.c.*" "test_${PROJECT_NAME}_\\2" tc_name ${tc_src})
  message(STATUS "adding test case ${tc_name} (using ${tc_src})")

  add_executable(${tc_name} ${tc_src})
  target_link_libraries(${tc_name} sdpa-com)
  target_link_libraries(${tc_name} sdpa-daemon)

if (USE_SMC_SC)
  	target_link_libraries(${tc_name} sdpa-jobFSM_SMC)
endif (USE_SMC_SC)

  target_link_libraries(${tc_name}
    sdpa-com
    sdpa-events
    sdpa-util
    sdpa
    seda
    ${FhgCom_LIBRARY}
    ${FhgLog_LIBRARY}
    ${Boost_UNIT_TEST_LIBRARIES}
    ${Boost_LIBRARIES}
    )
  get_target_property(LOC ${tc_name} LOCATION_)
  add_test(${tc_name} ${LOC})
endforeach(tc_src ${tc_srcs})

