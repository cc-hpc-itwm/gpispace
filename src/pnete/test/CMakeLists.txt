# bernd.loerwald@itwm.fraunhofer.de

  add_executable (monitor_pressure_generator monitor_pressure_generator.cpp)
  target_link_libraries (monitor_pressure_generator pnet fhglog ${Boost_THREAD_LIBRARY} Util::Generic)
  #! \todo Add test for monitor pressure.
