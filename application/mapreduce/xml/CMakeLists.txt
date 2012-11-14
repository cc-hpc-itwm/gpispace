include(pnet_macros)

pnet_compile( mapreduce mapreduce.xpnet
  INCLUDES ${CMAKE_SOURCE_DIR}/application/lib
  OUTPUT mapreduce.pnet
  GENERATE mapreduce
  INSTALL ${SDPA_APPS_DIR}/mapreduce
  CXXFLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/..
           -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/fvm-pc
           -I${CMAKE_SOURCE_DIR}/sdpa/plugin
           -I${CMAKE_SOURCE_DIR}/sdpa
           -O3
  BUILD
  COMPONENT applications
)
