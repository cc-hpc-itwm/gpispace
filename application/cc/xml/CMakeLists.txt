include(pnet_macros)

pnet_compile(sdpacc compile.xml
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
  COMPONENT applications
)
