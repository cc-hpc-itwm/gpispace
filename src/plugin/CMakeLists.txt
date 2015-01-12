set (core_srcs
  core/kernel.cpp
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
)

target_link_libraries (fhg-plugin ${FhgLog_LIBRARY})
target_link_libraries (fhg-plugin-shared ${FhgLog_LIBRARY})

target_link_libraries(fhg-plugin ${UTIL_LIBRARIES} ${CMAKE_DL_LIBS})
target_link_libraries(fhg-plugin-shared ${UTIL_LIBRARIES} ${CMAKE_DL_LIBS})
