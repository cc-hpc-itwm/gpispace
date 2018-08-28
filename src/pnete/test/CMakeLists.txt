extended_add_executable (NAME monitor_pressure_generator
  SOURCES monitor_pressure_generator.cpp
  LIBRARIES pnet fhglog Boost::thread Util::Generic
            gspc::tracing
)

#! \todo Add test for monitor pressure.
