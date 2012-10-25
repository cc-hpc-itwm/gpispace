# -*- mode: cmake; -*-

include(fhg_test_macros)
include(fhg_plugin_macros)

fhg_add_test(test_file_storage.cpp
  LINK_LIBRARIES fhg-plugin
  ARGS ${CMAKE_CURRENT_BINARY_DIR}/Testing/Temporary
  DESCRIPTION "Tests the file-storage backend"
  STANDALONE
)
fhg_add_test(test_null_storage.cpp
  LINK_LIBRARIES fhg-plugin
  DESCRIPTION "Tests the null-storage backend"
  STANDALONE
)

get_target_property(HELLO_PLUGIN_LOCATION hello-plugin LOCATION)
get_filename_component(BUILTIN_PLUGINS_PATH ${HELLO_PLUGIN_LOCATION} PATH)
fhg_add_test(test_kernel.cpp
  LINK_LIBRARIES fhg-plugin
  ARGS ${BUILTIN_PLUGINS_PATH}
  DESCRIPTION "Tests the plugin kernel"
  STANDALONE
)

fhg_add_plugin(test-good-magic good-magic.cpp)
fhg_add_plugin(test-bad-magic bad-magic.cpp)

fhg_add_test(test_plugin_version.cpp
  LINK_LIBRARIES fhg-plugin
  ARGS ${CMAKE_CURRENT_BINARY_DIR}
  DESCRIPTION "Tests the plugin kernel"
  STANDALONE
)
