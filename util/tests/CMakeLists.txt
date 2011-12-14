# -*- mode: cmake; -*-

include(fhg_test_macros)

fhg_add_test(test_file_storage.cpp
  LINK_LIBRARIES fhg-plugin
  ARGS ${CMAKE_CURRENT_BINARY_DIR}/Testing/Temporary
  DESCRIPTION "Tests the file-storage backend"
)
fhg_add_test(test_null_storage.cpp
  LINK_LIBRARIES fhg-plugin
  DESCRIPTION "Tests the null-storage backend"
)

fhg_add_test(test_semaphore.cpp
  DESCRIPTION "Tests the semaphore implementation"
)

fhg_add_test(test_event.cpp
  DESCRIPTION "Tests the thread event implementation"
)

fhg_add_test(test_ini_parser.cpp
  DESCRIPTION "Tests the ini-parser"
)

fhg_add_test(test_parent_path.cpp)

fhg_add_test(test_kernel.cpp
  LINK_LIBRARIES fhg-plugin
  ARGS ${CMAKE_BINARY_DIR}/util/fhg/plugin/builtin
  DESCRIPTION "Tests the plugin kernel"
)
