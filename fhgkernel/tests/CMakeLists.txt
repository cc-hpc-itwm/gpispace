# -*- mode: cmake; -*-

include(fhg_test_macros)
include(fhg_plugin_macros)

fhg_add_test(test_file_storage.cpp
  LINK_LIBRARIES fhg-plugin
  ARGS ${CMAKE_CURRENT_BINARY_DIR}/Testing/Temporary
  DESCRIPTION "Tests the file-storage backend"
)
fhg_add_test(test_null_storage.cpp
  LINK_LIBRARIES fhg-plugin
  DESCRIPTION "Tests the null-storage backend"
)

fhg_add_test(test_kernel.cpp
  LINK_LIBRARIES fhg-plugin
  ARGS ${CMAKE_BINARY_DIR}/util/fhg/plugin/builtin
  DESCRIPTION "Tests the plugin kernel"
)

fhg_add_plugin(test-good-magic good-magic.cpp)
fhg_add_plugin(test-bad-magic bad-magic.cpp)

fhg_add_test(test_plugin_version.cpp
  LINK_LIBRARIES fhg-plugin
  ARGS ${CMAKE_CURRENT_BINARY_DIR}
  DESCRIPTION "Tests the plugin kernel"
)
