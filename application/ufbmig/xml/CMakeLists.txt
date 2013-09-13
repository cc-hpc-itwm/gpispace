include(pnet_macros)

pnet_compile(ufbmig_prepare prepare.xml
  OUTPUT ufbmig_prepare.pnet
  GENERATE ufbmig_prepare
  CXXFLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/..
           -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/sdpa
           -O3
  BUILD
)

pnet_compile(ufbmig_init init.xml
  OUTPUT ufbmig_init.pnet
  GENERATE ufbmig_init
  CXXFLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/..
           -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/fvm-pc
           -I${CMAKE_SOURCE_DIR}/sdpa
           -I${CMAKE_SOURCE_DIR}/util
           -O3
  BUILD
)

#pnet_compile(ufbmig_mask update_salt_mask.xml
#   INCLUDES ${CMAKE_SOURCE_DIR}/application
#   OUTPUT ufbmig_mask.pnet
#   GENERATE ufbmig_mask
#   CXXFLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/..
#            -I${CMAKE_SOURCE_DIR}/fhglog
#            -I${CMAKE_BINARY_DIR}/fhglog
#            -I${CMAKE_SOURCE_DIR}/fvm-pc
#            -I${CMAKE_SOURCE_DIR}/sdpa
#            -O3
#   BUILD
#)

pnet_compile(ufbmig_calc calc.xml
  INCLUDES ${CMAKE_SOURCE_DIR}/application
  OUTPUT ufbmig_calc.pnet
  GENERATE ufbmig_calc
  CXXFLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/..
           -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/fvm-pc
           -I${CMAKE_SOURCE_DIR}/sdpa/plugin
           -I${CMAKE_SOURCE_DIR}/sdpa
           -I${CMAKE_SOURCE_DIR}/util
           -O3
  BUILD
)

pnet_compile(ufbmig_done finalize.xml
  OUTPUT ufbmig_done.pnet
  GENERATE ufbmig_done
  CXXFLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/..
           -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/fvm-pc
           -I${CMAKE_SOURCE_DIR}/util
           -I${CMAKE_SOURCE_DIR}/sdpa
           -O3
  BUILD
)
