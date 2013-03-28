# -*- mode: cmake; -*-
find_package (FVM REQUIRED QUIET)
include_directories(${FVM_INCLUDE_DIR})

include(fhg_plugin_macros)

fhg_add_plugin(gpi gpi.cpp
  EXPORTS GPI
  LINK_LIBRARIES
    gpi-space-pc-client-static
    gpi-space-pc-segment
    gpi-space-static
    ${FhgLog_LIBRARY_SHARED}
    ${Boost_SERIALIZATION_SHARED_LIBRARY}
    ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADERS gpi.hpp
  HEADER_DESTINATION include/fhg/plugins
  PLUGIN_DESTINATION libexec/fhg/plugins
  COMPONENT plugins
)

fhg_add_plugin(gpi_compat gpi_compat.cpp
  EXPORTS GPI
  LINK_LIBRARIES
    ${Boost_SERIALIZATION_SHARED_LIBRARY}
    fhg-util
  INSTALL
  PLUGIN_DESTINATION libexec/fhg/plugins
  COMPONENT plugins
)
