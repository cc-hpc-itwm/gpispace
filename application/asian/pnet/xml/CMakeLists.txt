include(pnet_macros)

pnet_compile(asian wrapper.xml
  OUTPUT asian.pnet
  GENERATE asian
  LDFLAGS "-L${CMAKE_BINARY_DIR}/application/example/process"
  CXXFLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/..
           -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/application/example
           -O3
  BUILD
  DEPENDS process
  INSTALL "${SDPA_APPS_DIR}/asian"
  COMPONENT applications
)
