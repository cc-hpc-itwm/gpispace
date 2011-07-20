configure_file(${CMAKE_CURRENT_SOURCE_DIR}/build.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/build.hpp @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
set(hdrs
  ${CMAKE_CURRENT_BINARY_DIR}/build.hpp
  config.hpp
  plugin_base.hpp
  plugin.hpp
  kernel.hpp
)

add_subdirectory(core)

set(fhg-plugin-static-libs fhg-plugin-core)

add_subdirectory(builtin)
list(APPEND fhg-plugin-static-libs fhg-builtin-plugins)
list(APPEND fhg-plugin-static-libs ${FhgLog_LIBRARY} ${Boost_LIBRARIES})

message(STATUS "***************** ${fhg-plugin-static-libs}")

include(libutils)
merge_static_libs(fhg-plugin fhg-plugin "${fhg-plugin-static-libs}")

target_link_libraries(fhg-plugin dl)

install (FILES ${hdrs}
  DESTINATION include/fhg/plugin
  COMPONENT headers
)
