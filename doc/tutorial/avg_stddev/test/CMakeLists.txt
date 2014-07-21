# mirko.rahn@itwm.fraunhofer.de

fhg_add_application_test (tutorial_avg_stddev_we-exec SCRIPT test.we-exec)
fhg_add_application_test (tutorial_avg_stddev_submit SCRIPT test.submit
  RESOURCE_LOCK starts_drts_environment
)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_executable (gen.bin gen.cpp)
target_link_libraries (gen.bin
  ${Boost_PROGRAM_OPTIONS_LIBRARIES}
  ${Boost_THREAD_LIBRARIES}
  ${Boost_SYSTEM_LIBRARIES}
  pthread
)
