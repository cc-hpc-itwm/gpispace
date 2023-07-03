add_library (asianopt-base OBJECT "asianopt.cpp")
set_property (TARGET asianopt-base PROPERTY POSITION_INDEPENDENT_CODE ON)

swh_add_implementation_module (NAME asianopt
  SOURCES "asianopt-wrapper.cpp"
  $<TARGET_OBJECTS:asianopt-base>
)

swh_add_implementation_exe (NAME asianopt
  SOURCES "asianopt-run.cpp"
  $<TARGET_OBJECTS:asianopt-base>
)

add_executable (asianopt-runs.test
  "test/asianopt.cpp" $<TARGET_OBJECTS:asianopt-base>
)
target_compile_definitions (asianopt-runs.test PRIVATE
  "-DBOOST_TEST_MODULE=\"asianopt-runs\""
)
target_link_libraries (asianopt-runs.test PRIVATE Boost::unit_test_framework)
add_test (NAME asianopt-runs COMMAND $<TARGET_FILE:asianopt-runs.test>)


swh_add_implementation_module (NAME barropt
  SOURCES "barropt.cpp"
)
swh_add_implementation_exe (NAME barropt
  SOURCES "barropt-run.cpp"
)
