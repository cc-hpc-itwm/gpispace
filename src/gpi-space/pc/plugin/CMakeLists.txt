include_directories(${FVM_INCLUDE_DIR})

include(fhg_plugin_macros)

fhg_add_plugin(gpi gpi.cpp
  LINK_LIBRARIES
    gpi-space-pc-client-static
    gpi-space-pc-segment
    ${Boost_SERIALIZATION_LIBRARY}
  INSTALL
)

fhg_add_plugin(gpi_compat gpi_compat.cpp
  LINK_LIBRARIES
    ${Boost_SERIALIZATION_LIBRARY}
    fhg-util
  INSTALL
)
