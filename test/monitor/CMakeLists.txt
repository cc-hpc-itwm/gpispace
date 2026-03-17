gspc_add_executable (NAME monitor_pressure_generator
  SOURCES monitor_pressure_generator.cpp
  LIBRARIES gspc_we
            gspc_scheduler
            gspc_logging
            gspc_util
            Boost::unit_test_framework # util-generic#61
            gspc_testing
)

#! \todo Add test for monitor pressure.
