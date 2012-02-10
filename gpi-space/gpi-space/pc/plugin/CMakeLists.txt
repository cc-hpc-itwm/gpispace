# -*- mode: cmake; -*-
find_package (FVM REQUIRED)
include_directories(${FVM_INCLUDE_DIR})

include(fhg_plugin_macros)

fhg_add_plugin(gpi gpi.cpp
  EXPORTS GPI
  LINK_LIBRARIES
    gpi-space-pc-client-static
    gpi-space-pc-segment
    gpi-space-static
    ${Boost_SERIALIZATION_SHARED_LIBRARY}
  INSTALL
  HEADERS gpi.hpp
  HEADER_DESTINATION include/plugins
  PLUGIN_DESTINATION libexec/plugins
  COMPONENT plugins
)

fhg_add_plugin(gpi_compat gpi_compat.cpp
  EXPORTS GPI
  LINK_LIBRARIES
    ${Boost_SERIALIZATION_SHARED_LIBRARY}
  INSTALL
  PLUGIN_DESTINATION libexec/plugins
  COMPONENT plugins
)
