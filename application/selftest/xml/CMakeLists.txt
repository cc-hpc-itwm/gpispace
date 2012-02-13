include(pnet_macros)

pnet_compile(selftest selftest.xml
  OUTPUT selftest.pnet
  GENERATE selftest
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -O3
  BUILD
  INSTALL libexec/apps/selftest
  COMPONENT applications
)

install(FILES selftest.xml
  DESTINATION libexec/apps/selftest
)
