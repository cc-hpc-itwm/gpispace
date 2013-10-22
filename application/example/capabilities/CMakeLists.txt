include(pnet_macros)

pnet_compile(capabilities capabilities.xpnet
  OUTPUT capabilities.pnet
  GENERATE capabilities
  INSTALL ${SDPA_APPS_DIR}/capabilities
  INCLUDES ${CMAKE_SOURCE_DIR}/application/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/application/example
           -O3
  BUILD
  COMPONENT applications
)
