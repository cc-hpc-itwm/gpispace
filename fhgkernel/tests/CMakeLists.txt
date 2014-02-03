include(fhg_plugin_macros)

fhg_add_plugin(test-good-magic good-magic.cpp)
fhg_add_plugin(test-bad-magic bad-magic.cpp)

fhg_add_test(plugin_version.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhg-plugin ${Boost_FILESYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY}
  ARGS ${CMAKE_CURRENT_BINARY_DIR}
)
