include(pnet_macros)

configure_file(compile.xml.in ${CMAKE_CURRENT_BINARY_DIR}/compile.xml @ONLY)

pnet_compile(sdpacc ${CMAKE_CURRENT_BINARY_DIR}/compile.xml
  OUTPUT compile.pnet
  GENERATE sdpacc
  INSTALL libexec/apps/sdpacc
  LDFLAGS "-L${CMAKE_BINARY_DIR}/application/example/process"
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/application/example
           -O3
  DEPENDS process
  BUILD
)
