include(pnet_macros)

pnet_compile (kdm kdm.xpnet
  INCLUDES ${CMAKE_SOURCE_DIR}/application/lib
  OUTPUT kdm.pnet
  GENERATE kdm
  INSTALL ${SDPA_APPS_DIR}/kdm
  CXXFLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/..
           -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/fvm-pc
           -I${CMAKE_SOURCE_DIR}/sdpa/plugin
           -I${CMAKE_SOURCE_DIR}/sdpa
           -O3
#  BUILD
  COMPONENT applications
)

pnet_compile (kdmopt kdmopt.xpnet
  INCLUDES ${CMAKE_SOURCE_DIR}/application/lib
  OUTPUT kdmopt.pnet
  GENERATE kdm
  INSTALL ${SDPA_APPS_DIR}/kdm
  CXXFLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/..
           -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/fvm-pc
           -I${CMAKE_SOURCE_DIR}/sdpa/plugin
           -I${CMAKE_SOURCE_DIR}/sdpa
           -O3
#  BUILD
  COMPONENT applications
)
