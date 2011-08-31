find_package (FVM REQUIRED)
include_directories(${FVM_INCLUDE_DIR})

include(fhg_plugin_macros)

fhg_add_plugin(gpi gpi.cpp
  EXPORTS GPI
  LINK_LIBRARIES
    gpi-space-pc-client-static
    gpi-space-pc-segment
    gpi-space-static
    $ENV{BOOST_ROOT}/lib/libboost_serialization.so
)

fhg_add_plugin(gpi-compat gpi-compat.cpp
  EXPORTS GPI
  LINK_LIBRARIES
    $ENV{BOOST_ROOT}/lib/libboost_serialization.so
)

install(FILES gpi.hpp DESTINATION include/plugins COMPONENT headers)
install(TARGETS gpi-plugin LIBRARY
  DESTINATION libexec/plugins
  COMPONENT plugins
)
