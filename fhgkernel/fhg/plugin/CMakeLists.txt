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

add_subdirectory(core)

set(fhg-plugin-static-libs fhg-plugin-core)

add_subdirectory(builtin)
list(APPEND fhg-plugin-static-libs fhg-builtin-plugins)
list(APPEND fhg-plugin-static-libs ${FhgLog_LIBRARY} ${Boost_LIBRARIES})

include(libutils)
merge_static_libs(fhg-plugin fhg-plugin "${fhg-plugin-static-libs}")

target_link_libraries(fhg-plugin ${UTIL_LIBRARIES} ${CMAKE_DL_LIBS})

install (FILES ${hdrs}
  DESTINATION include/fhg/plugin
  COMPONENT headers
)
