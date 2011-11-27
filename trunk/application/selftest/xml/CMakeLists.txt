include(pnet_macros)

pnet_compile(selftest selftest.xml
  OUTPUT selftest.pnet
  GENERATE selftest
  INSTALL libexec/apps/selftest
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -O3
  BUILD
)

install(FILES selftest.xml
  DESTINATION libexec/apps/selftest
)
