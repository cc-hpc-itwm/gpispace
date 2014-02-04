include_directories(${CMAKE_CURRENT_BINARY_DIR})
set(hdrs
  descriptor.hpp
  plugin.hpp
  kernel.hpp
)

set (core_srcs
  core/plugin.cpp
  core/kernel.cpp
  core/license.cpp
)

add_library(fhg-plugin ${core_srcs})
add_library(fhg-plugin-shared SHARED ${core_srcs})
target_link_libraries(fhg-plugin ${Boost_FILESYSTEM_LIBRARY})
set_target_properties(fhg-plugin-shared
  PROPERTIES OUTPUT_NAME fhg-plugin
             VERSION     1
)

install (TARGETS fhg-plugin-shared
  LIBRARY
  DESTINATION lib
  COMPONENT runtime
)

target_link_libraries (fhg-plugin ${FhgLog_LIBRARY})
target_link_libraries (fhg-plugin-shared ${FhgLog_LIBRARY})

if (ENABLE_LICENSE_CHECK)
  target_link_libraries (fhg-plugin ${NACL_LIBRARY})
  target_link_libraries (fhg-plugin-shared ${NACL_LIBRARY})

  set_target_properties(fhg-plugin        PROPERTIES COMPILE_DEFINITIONS "FHG_ENABLE_LICENSE_CHECK")
  set_target_properties(fhg-plugin-shared PROPERTIES COMPILE_DEFINITIONS "FHG_ENABLE_LICENSE_CHECK")
endif()

target_link_libraries(fhg-plugin ${UTIL_LIBRARIES} ${CMAKE_DL_LIBS})
target_link_libraries(fhg-plugin-shared ${UTIL_LIBRARIES} ${CMAKE_DL_LIBS})

install (FILES ${hdrs}
  DESTINATION include/fhg/plugin
  COMPONENT headers
)
