include(cmake_parse_arguments)
include(car_cdr_macros)

macro(FHG_ADD_TEST)
  PARSE_ARGUMENTS(TEST
    "LINK_LIBRARIES;DEPENDS;PROJECT;ARGS;DESCRIPTION"
    "QUIET;STANDALONE"
    ${ARGN}
    )
  CAR(TEST_SOURCE ${TEST_DEFAULT_ARGS})
  CDR(TEST_ADDITIONAL_SOURCES ${TEST_DEFAULT_ARGS})

  if (NOT TEST_PROJECT)
    set(TEST_PROJECT "${PROJECT_NAME}")
  endif()

  if (NOT STANDALONE)
    set(TEST_LINK_LIBRARIES ${TEST_LINK_LIBRARIES} ${Boost_UNIT_TEST_LIBRARIES} ${Boost_LIBRARIES})
  endif()

  # get the filename without extension
  string(REGEX REPLACE "(.*/)?(.*)\\.c.*" "${TEST_PROJECT}_\\2" tc_name ${TEST_SOURCE})

  if (NOT TEST_QUIET)
    message(STATUS "**** Adding test ${tc_name} ${TEST_ARGS} (${TEST_DESCRIPTION})")
  endif()

  add_executable(${tc_name} ${TEST_SOURCE} ${TEST_ADDITIONAL_SOURCES})
  target_link_libraries(${tc_name} ${TEST_LINK_LIBRARIES})
  get_target_property(TC_LOC ${tc_name} LOCATION)
  add_test (${tc_name} ${TC_LOC} ${TEST_ARGS})

  foreach (d ${TEST_DEPENDS})
    add_dependencies(${tc_name} ${d})
  endforeach()
endmacro()
