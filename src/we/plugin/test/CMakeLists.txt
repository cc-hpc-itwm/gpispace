if (BUILD_TESTING)
  gspc_add_library (NAME A
    NAMESPACE PluginTest
    SOURCES "A.cpp"
    TYPE SHARED
    LIBRARIES GPISpace::header-only
              we-dev
  )
  gspc_add_library (NAME B
    NAMESPACE PluginTest
    SOURCES "B.cpp"
    TYPE SHARED
    LIBRARIES GPISpace::header-only
              we-dev
  )
  gspc_add_library (NAME C
    NAMESPACE PluginTest
    SOURCES "C.cpp"
    TYPE SHARED
    LIBRARIES GPISpace::header-only
              we-dev
  )
  gspc_add_library (NAME D
    NAMESPACE PluginTest
    SOURCES "D.cpp"
    TYPE SHARED
    LIBRARIES GPISpace::header-only
              we-dev
  )
endif()

fhg_add_test (NAME we_Plugins
  SOURCES test.cpp
  USE_BOOST
  ARGS $<TARGET_FILE:PluginTest::A>
       $<TARGET_FILE:PluginTest::B>
       $<TARGET_FILE:PluginTest::C>
       $<TARGET_FILE:PluginTest::D>
  LIBRARIES gspc
            Util::Generic
  DEPENDS PluginTest::A
          PluginTest::B
          PluginTest::C
          PluginTest::D
)

fhg_add_test (NAME "we_plugin_exception_from_plugin_ctor_is_transported_to_users"
  SOURCES "exception_from_plugin_ctor_is_transported_to_users/test.cpp"
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --plugin-path "$<TARGET_FILE:PluginTest::D>"
  LIBRARIES gspc
            test-utilities
            Util::Generic
            Boost::filesystem
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
