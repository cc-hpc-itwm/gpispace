include(pnet_macros)

pnet_compile(capabilities_no_mandatory capabilities_no_mandatory.xml
  OUTPUT capabilities_no_mandatory.pnet
  GENERATE capabilities_no_mandatory
  INSTALL ${SDPA_APPS_DIR}/capabilities_no_mandatory
  INCLUDES ${CMAKE_SOURCE_DIR}/application/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/application/example
           -O3
  BUILD
  COMPONENT applications
)
