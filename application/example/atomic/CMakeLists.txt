include(pnet_macros)

pnet_compile(atomic atomic.xpnet
  OUTPUT atomic.pnet
  GENERATE atomic
  INSTALL ${SDPA_APPS_DIR}/atomic
  INCLUDES ${CMAKE_SOURCE_DIR}/application/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/application/example
           -O3
  BUILD
  COMPONENT applications
)
