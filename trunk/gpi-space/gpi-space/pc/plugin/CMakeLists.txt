include(fhg_plugin_macros)

#
#SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#

fhg_add_plugin(gpi gpi.cpp)
target_link_libraries(gpi-plugin
  gpi-space-pc-client-static
  gpi-space-pc-segment
  gpi-space-static
  $ENV{BOOST_ROOT}/lib/libboost_serialization.so
)

install(FILES gpi.hpp DESTINATION include/plugins COMPONENT headers)
install(TARGETS gpi-plugin LIBRARY
  DESTINATION libexec/plugins
  COMPONENT plugins
  )
