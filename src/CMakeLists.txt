if (GSPC_WITH_IML)
  add_subdirectory (iml)
endif()

add_subdirectory (assert)
add_subdirectory (com)
add_subdirectory (configuration)
add_subdirectory (util)
add_subdirectory (xml)

if (GSPC_WITH_MONITOR_APP)
  add_subdirectory (monitor)
endif()

file (READ
  "${PROJECT_SOURCE_DIR}/include/gspc/we/type/signature/pnetc_compare.hpp"
  PNETC_COMPARE_HPP
)

configure_file ("we/type/signature/cpp.cpp.in"
  "${CMAKE_BINARY_DIR}/we/type/signature/cpp.cpp"
  @ONLY
)
configure_file ("${CMAKE_SOURCE_DIR}/test/we/type/signature.cpp.in"
  "${CMAKE_BINARY_DIR}/we/type/test/signature.cpp"
  @ONLY
)

gspc_add_library (NAME gspc_api_guard
  TYPE SHARED
  SOURCES
    "we/loader/api-guard.cpp"
  INCLUDE_DIRECTORIES
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
  LIBRARIES
    PUBLIC GPISpace::header-only
    PUBLIC Boost::boost
  INSTALL
    CREATE_BUNDLE_INFO
)

add_library (GPISpace::api_guard ALIAS gspc_api_guard)

add_subdirectory (scheduler_types)
add_subdirectory (we)

add_subdirectory (drts)
add_subdirectory (bin)

add_subdirectory (rpc)

add_subdirectory (rif)
add_subdirectory (scheduler)

file (WRITE "${CMAKE_BINARY_DIR}/gspc_version" "${PROJECT_VERSION}")
install (FILES "${CMAKE_BINARY_DIR}/gspc_version" DESTINATION .)

add_subdirectory (logging)
