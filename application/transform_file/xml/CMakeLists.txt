include(pnet_macros)

pnet_compile( transform_file transform_file.xml
  INCLUDES ${CMAKE_SOURCE_DIR}/application/lib
  OUTPUT transform_file.pnet
  GENERATE transform_file
  CXXFLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/..
           -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/fvm-pc
           -I${CMAKE_SOURCE_DIR}/sdpa/plugin
           -I${CMAKE_SOURCE_DIR}/sdpa
           -O3
  BUILD
)

