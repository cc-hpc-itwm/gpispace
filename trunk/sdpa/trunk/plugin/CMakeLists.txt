include(fhg_plugin_macros)

#
#SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#

# distributed runtime system plugin, aka interface to SDPA agent
fhg_add_plugin(drts drts.cpp)
target_link_libraries(drts-plugin
  sdpa-events
  sdpa-util
  sdpa
  ${FhgCom_LIBRARY}
  ${BOOST_ROOT}/lib/libboost_serialization.so
)

install(FILES drts.hpp DESTINATION include/plugins COMPONENT plugins)
install(TARGETS drts-plugin LIBRARY
  DESTINATION libexec/plugins
  COMPONENT plugins
  )
