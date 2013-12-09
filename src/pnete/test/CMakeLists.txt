# bernd.loerwald@itwm.fraunhofer.de

if (BUILD_TESTING)
  add_executable (monitor_pressure_generator monitor_pressure_generator.cpp)
  target_link_libraries (monitor_pressure_generator pnet ${FhgLog_LIBRARY} ${Boost_UNIT_TEST_LIBRARIES} ${Boost_LIBRARIES})
  #! \todo Add test for monitor pressure.
endif()
