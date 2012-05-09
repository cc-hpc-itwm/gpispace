include(pnet_macros)

pnet_compile(inotify inotify.xml
  OUTPUT inotify.pnet
  GENERATE inotify
  INSTALL ${SDPA_APPS_DIR}/inotify
  INCLUDES ${CMAKE_SOURCE_DIR}/application/lib
  CXXFLAGS -I${CMAKE_SOURCE_DIR}/fhglog
           -I${CMAKE_BINARY_DIR}/fhglog
           -I${CMAKE_SOURCE_DIR}/application/example
           -O3
  BUILD
  COMPONENT applications
)
