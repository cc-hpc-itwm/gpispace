include(fhg_plugin_macros)

fhg_add_test(file_storage.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhg-plugin ${Boost_FILESYSTEM_LIBRARY} ${Boost_SERIALIZATION_LIBRARY}
  ARGS ${CMAKE_CURRENT_BINARY_DIR}/Testing/Temporary
)
fhg_add_test(null_storage.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhg-plugin ${Boost_SERIALIZATION_LIBRARY}
)

get_target_property(HELLO_PLUGIN_LOCATION hello-plugin LOCATION)
get_filename_component(BUILTIN_PLUGINS_PATH ${HELLO_PLUGIN_LOCATION} PATH)
fhg_add_test(kernel.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhg-plugin ${Boost_FILESYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY}
  ARGS ${BUILTIN_PLUGINS_PATH}
)

fhg_add_plugin(test-good-magic good-magic.cpp)
fhg_add_plugin(test-bad-magic bad-magic.cpp)

fhg_add_test(plugin_version.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhg-plugin ${Boost_FILESYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY}
  ARGS ${CMAKE_CURRENT_BINARY_DIR}
)
