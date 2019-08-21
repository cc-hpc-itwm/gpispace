extended_add_library (NAME A
  NAMESPACE PluginTest
  SOURCES "A.cpp"
  POSITION_INDEPENDENT
  TYPE SHARED
  LIBRARIES pnet
)
extended_add_library (NAME B
  NAMESPACE PluginTest
  SOURCES "B.cpp"
  POSITION_INDEPENDENT
  TYPE SHARED
  LIBRARIES pnet
)
extended_add_library (NAME C
  NAMESPACE PluginTest
  SOURCES "C.cpp"
  POSITION_INDEPENDENT
  TYPE SHARED
  LIBRARIES pnet
)
extended_add_library (NAME D
  NAMESPACE PluginTest
  SOURCES "D.cpp"
  POSITION_INDEPENDENT
  TYPE SHARED
  LIBRARIES pnet
)
fhg_add_test (NAME Plugins
  SOURCES test.cpp
  USE_BOOST
  ARGS $<TARGET_FILE:PluginTest::A>
       $<TARGET_FILE:PluginTest::B>
       $<TARGET_FILE:PluginTest::C>
       $<TARGET_FILE:PluginTest::D>
  LIBRARIES gspc Util::Generic
)
