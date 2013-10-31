configure_file(${CMAKE_CURRENT_SOURCE_DIR}/magic.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/magic.hpp @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
set(hdrs
  ${CMAKE_CURRENT_BINARY_DIR}/magic.hpp
  descriptor.hpp
  config.hpp
  plugin_base.hpp
  plugin.hpp
  kernel.hpp
  capability.hpp
  storage.hpp
)

set (core_srcs
  core/plugin.cpp
  core/kernel.cpp
  core/plugin_kernel_mediator.cpp
  core/file_storage.cpp
  core/null_storage.cpp
  core/license.cpp
)

add_library(fhg-plugin-core ${core_srcs})
add_library(fhg-plugin-shared SHARED ${core_srcs})
target_link_libraries(fhg-plugin-shared ${Boost_FILESYSTEM_SHARED_LIBRARY})
set_target_properties(fhg-plugin-shared
  PROPERTIES OUTPUT_NAME fhg-plugin
             VERSION     1
)

install (TARGETS fhg-plugin-shared
  LIBRARY
  DESTINATION lib
  COMPONENT runtime
)

set (fhg-plugin-static-libs fhg-plugin-core)

add_subdirectory(builtin)
list(APPEND fhg-plugin-static-libs fhg-builtin-plugins)
list(APPEND fhg-plugin-static-libs ${FhgLog_LIBRARY} ${Boost_LIBRARIES})

if (ENABLE_LICENSE_CHECK)
  list(APPEND fhg-plugin-static-libs ${NACL_LIBRARY})
  set_target_properties(fhg-plugin-shared PROPERTIES COMPILE_DEFINITIONS "FHG_ENABLE_LICENSE_CHECK")
  set_target_properties(fhg-plugin-core   PROPERTIES COMPILE_DEFINITIONS "FHG_ENABLE_LICENSE_CHECK")
endif()

include (libutils)
merge_static_libs(fhg-plugin fhg-plugin "${fhg-plugin-static-libs}")

target_link_libraries(fhg-plugin ${UTIL_LIBRARIES} ${CMAKE_DL_LIBS})

install (FILES ${hdrs}
  DESTINATION include/fhg/plugin
  COMPONENT headers
)
