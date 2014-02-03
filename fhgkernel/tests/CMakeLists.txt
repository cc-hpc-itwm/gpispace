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

fhg_add_plugin(test-good-magic good-magic.cpp)
fhg_add_plugin(test-bad-magic bad-magic.cpp)

fhg_add_test(plugin_version.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhg-plugin ${Boost_FILESYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY}
  ARGS ${CMAKE_CURRENT_BINARY_DIR}
)
