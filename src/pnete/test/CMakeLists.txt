if (BUILD_TESTING)
  gspc_add_executable (NAME monitor_pressure_generator
    SOURCES monitor_pressure_generator.cpp
    LIBRARIES pnet
              gspc::logging
              Util::Generic
              Boost::unit_test_framework # util-generic#61
  )

#! \todo Add test for monitor pressure.
endif()
