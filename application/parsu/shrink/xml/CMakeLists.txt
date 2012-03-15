
add_subdirectory (generate)

set (xml
   shrink.xml.in
)

install(FILES ${xml}
  DESTINATION ${PARSU_SHRINK}
  COMPONENT applications)

# include(pnet_macros)

# pnet_compile(parsu_shrink shrink.xml.in
#   INCLUDES generate
#            ${CMAKE_SOURCE_DIR}/application/lib
#   GENERATE parsu_shrink
#   LDFLAGS "-L${CMAKE_BINARY_DIR}/application/example/process"
#   CXXFLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/../util
#            -I${CMAKE_SOURCE_DIR}/fhglog
#            -I${CMAKE_BINARY_DIR}/fhglog
#            -I${CMAKE_SOURCE_DIR}/application/example
#            -O3
#   BUILD
#   DEPENDS process
#   INSTALL libexec/apps/parsu/shrink
#)
