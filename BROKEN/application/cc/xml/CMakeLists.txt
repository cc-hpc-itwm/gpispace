include(pnet_macros)

pnet_compile(sdpacc compile.xml
  OUTPUT compile.pnet
  GENERATE sdpacc
  LDFLAGS "-L${CMAKE_BINARY_DIR}/share/process"
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/share
           -O3
  DEPENDS process
  BUILD
  COMPONENT applications
)
