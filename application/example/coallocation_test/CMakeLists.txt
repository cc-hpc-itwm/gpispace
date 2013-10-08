include(pnet_macros)

pnet_compile(coallocation_test coallocation_test.xml
  OUTPUT coallocation_test.pnet
  GENERATE coallocation_test
  INSTALL ${SDPA_APPS_DIR}/coallocation_test
  INCLUDES ${CMAKE_SOURCE_DIR}/application/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/application/example
           -O3
  BUILD
  COMPONENT applications
)
