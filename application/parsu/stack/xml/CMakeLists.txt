
add_subdirectory (generate)

set (xml
   stack.xml.in
)

install(FILES ${xml}
  DESTINATION ${PARSU_STACK}
  COMPONENT applications)

# include(pnet_macros)

# pnet_compile(parsu_stack stack.xml.in
#   INCLUDES generate
#            ${CMAKE_SOURCE_DIR}/application/lib
#   GENERATE parsu_stack
#   LDFLAGS "-L${CMAKE_BINARY_DIR}/application/example/process"
#   CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
#            -I${CMAKE_BINARY_DIR}/fhglog
#            -I${CMAKE_SOURCE_DIR}/application/example
#            -O3
#   BUILD
#   DEPENDS process
#   INSTALL libexec/apps/parsu/stack
# )
